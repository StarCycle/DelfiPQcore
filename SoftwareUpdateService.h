/*
 * SoftwareUpdateService.h
 *
 *  Created on: 27 Jul 2019
 *      Author: Jasper Haenen
 */

#ifndef SOFTWAREUPDATESERVICE_H_
#define SOFTWAREUPDATESERVICE_H_

#include "Service.h"
#include "Console.h"
#include "MB85RS.h"
#include "DataMessage.h"
#include "Bootloader.h"
#include "FRAMMap.h"

extern "C" {
    #include "md5.h"
}

#define SOFTWAREUPDATE_SERVICE              18
//#define SOFTWAREUPDATE_SERVICE_FRAM_OFFSET  0
#define SOFTWAREUPDATE_SERVICE_FRAM_SIZE    2 * (METADATA_SIZE) + 2 * (PAR_CRC_SIZE) + UPDATE_PROGRESS_SIZE

#define FRAM_SIZE               32768
#define BLOCK_SIZE              32
#define MD5_SIZE                16
#define BANK_SIZE               131072
#define SLOT_SIZE               (BANK_SIZE / 2)
#define SECTORS_PER_SLOT        16
#define SECTOR_SIZE             (SLOT_SIZE / SECTORS_PER_SLOT)
#define NUM_SLOTS               2
#define PAR_CRC_SIZE            (SLOT_SIZE / BLOCK_SIZE)
//#define METADATA_SIZE           (MD5_SIZE + 8 + 2 + 1)
#define MAX_BLOCK_AMOUNT        (SLOT_SIZE / BLOCK_SIZE)
#define BYTE_SIZE               8

//sizes of FRAM Data and locations
#define METADATA_STATE_SIZE         1
#define METADATA_VERSION_SIZE       8 //<-- this can be shortened
#define METADATA_NR_OF_BLOCKS_SIZE  2
#define METADATA_SIZE               METADATA_STATE_SIZE + MD5_SIZE + METADATA_VERSION_SIZE + METADATA_NR_OF_BLOCKS_SIZE

////
// METADATA:
// 1  Byte       STATE (EMPTY/PARTIAL/FULL)
// 16 Bytes      MD5
// 2  Bytes      VERSION NR
// 2  Bytes      (LSB , MSB)
////

#define SLOT1_METADATA              SOFTWAREUPDATE_SERVICE_FRAM_OFFSET
#define SLOT1_METADATA_STATE        SLOT1_METADATA
#define SLOT1_METADATA_MD5          SLOT1_METADATA_STATE + METADATA_STATE_SIZE
#define SLOT1_METADATA_VERSION      SLOT1_METADATA_MD5 + MD5_SIZE
#define SLOT1_METADATA_NR_OF_BLOCKS SLOT1_METADATA_VERSION + METADATA_VERSION_SIZE
#define SLOT1_PAR_CRC               SLOT1_METADATA + METADATA_SIZE
#define SLOT1_CRC_CHECKLIST         SLOT1_PAR_CRC + PAR_CRC_SIZE
#define SLOT1_BLOCK_CHECKLIST       SLOT1_CRC_CHECKLIST + (MAX_BLOCK_AMOUNT/8)

#define SLOT2_METADATA              SLOT1_BLOCK_CHECKLIST + (MAX_BLOCK_AMOUNT/8)
#define SLOT2_METADATA_STATE        SLOT2_METADATA
#define SLOT2_METADATA_MD5          SLOT2_METADATA_STATE + METADATA_STATE_SIZE
#define SLOT2_METADATA_VERSION      SLOT2_METADATA_MD5 + MD5_SIZE
#define SLOT2_METADATA_NR_OF_BLOCKS SLOT2_METADATA_VERSION + METADATA_VERSION_SIZE
#define SLOT2_PAR_CRC               SLOT2_METADATA + METADATA_SIZE
#define SLOT2_CRC_CHECKLIST         SLOT1_PAR_CRC + PAR_CRC_SIZE
#define SLOT2_BLOCK_CHECKLIST       SLOT1_CRC_CHECKLIST + (MAX_BLOCK_AMOUNT/8)
////
// UPDATE_PROGRESS:
// 1                            Byte       STATE (EMPTY/PARTIAL/FULL)
// MAX_BLOCK_AMOUNT/INT_SIZE    Bytes      CRC Bitwise checklist
// MAX_BLOCK_AMOUNT/INT_SIZE    Bytes      BLOCK Bitwise checklist
// 2  Bytes      (LSB , MSB)
////
////
// STATE BYTE:
//  76543210
// [10] : Empty(00), Partial(01), Full(10)
// [2]  : UpdateFlag(100)(0x04)
// [3]  : EraseFlag(1000)(0x08)
// [4]  : MetaDataReceived(1 0000) (0x10)
// [5]  : PartialCRCReceived(10 0000) (0x20)
// [6]  : MD5_CORRECT Flag(100 0000) (0x40)
// [7]  : SlotUpdating (0x80) slot1(0) or slot2(1)
//

#define BANK1_ADDRESS           0x20000

#define ACKNOWLEDGE             13



enum slot_status{
    EMPTY = 0x00,
    PARTIAL = 0x01,
    FULL = 0x02
};
enum flags {
    ERASE_FLAG = 0x08,
    UPDATE_FLAG = 0x04,
    METADATA_FLAG = 0x10,
    PARTIAL_CRC_FLAG = 0x20,
    MD5_CORRECT_FLAG = 0x40,
    SLOT_SELECT_FLAG = 0x80
};

enum commands{
    START_OTA,
    SET_METADATA,
    GET_METADATA,
    SET_PARTIAL_CRCS,
    SEND_MISSED_PARTIALS,
    SET_BLOCK,
    CHECK_MD5,
    STOP_OTA,
    ERASE_SLOT,
    SET_BOOT_SLOT,
    GET_NUM_MISSED_BLOCKS,
    GET_MISSED_BLOCKS,
    GET_MISSED_CRC,
    GET_VERSION_NUMBER
};

enum command_offsets {
    COMMAND_METHOD,
    COMMAND_DATA
};

enum error_codes{
    NO_ERROR,
    NO_FRAM_ACCESS,
    NO_SLOT_ACCESS,
    MEMORY_FULL,
    PARAMETER_MISMATCH,
    UPDATE_NOT_STARTED,
    UPDATE_ALREADY_STARTED,
    UPDATE_NOT_CURRENT_SESSION,
    UPDATE_TO_BIG,
    METADATA_ALREADY_RECEIVED,
    METADATA_NOT_RECEIVED,
    PARTIAL_ALREADY_RECEIVED,
    PARTIAL_NOT_RECEIVED,
    CRC_MISMATCH,
    MD5_MISMATCH,
    SLOT_OUT_OF_RANGE,
    OFFSET_OUT_OF_RANGE,
    SLOT_NOT_EMPTY,
    SLOT_NOT_PROGRAMMED,
    SELF_ACTION,
    NO_VERSION_NUMBER
};

enum metadata_offset {
    STATUS_OFFSET,
    CRC_OFFSET,
    VERSION_OFFSET = CRC_OFFSET + MD5_SIZE,
    NUM_BLOCKS_OFFSET = VERSION_OFFSET + 8
};

static const uint8_t CRC_TABLE[256] = { //CRC8-CCITT table, polynomial x^8+x^2+x+1
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

class SoftwareUpdateService: public Service
{
 public:
     SoftwareUpdateService(MB85RS &fram_in);
     SoftwareUpdateService(MB85RS &fram_in, uint8_t * versionString);
     virtual bool process( DataMessage &command, DataMessage &workingBuffer );
 private:
     void startOTA(unsigned char slot_number, bool allow_resume);

     void setMetadata(unsigned char* metadata);
     void getMetadata(unsigned char slot_number);

     void setPartialCRCs(unsigned char* crc_block, unsigned char num_bytes, unsigned short crc_offset);
     void setBlock(unsigned char* data_block, unsigned short block_offset);
     bool checkPartialCRC(unsigned char* data_block, unsigned short block_offset);

     void checkMD5(unsigned char slot_number);

     void stopOTA();

     void eraseSlot(unsigned char slot);

     void setBootSlot(unsigned char slot, bool permanent);

     unsigned int  getNumOfMissedBlocks();
     unsigned int getNumOfMissedCRCs();
     void getMissedBlocks();
     void getMissedCRCs();


     void print_metadata(unsigned char* metadata);
     void throw_error(unsigned char error);

     bool hasVersionNumber;
     uint8_t versionNumber[8] = {0};
     void getVersionNumber();

     unsigned char state_flags = 0; //current state of update
     unsigned char update_slot = 0; //target of update
     unsigned char slot_erase = 0;  //flag for erasure
     unsigned short num_update_blocks = 0; //number of expected blocks

     unsigned char* payload_data = 0;
     unsigned char payload_size = 0;

     unsigned char blocks_received_buffer[MAX_BLOCK_AMOUNT/8] = { 0 };  //checklist for blocks
     unsigned char crc_received_buffer[MAX_BLOCK_AMOUNT/8] = { 0 };     //checklist for CRCs
     //unsigned char crcs_buffer[PAR_CRC_SIZE] = { 0 };                   //copy of CRCs in RAM
     //unsigned char metadata_buffer[METADATA_SIZE] = { 0 };              //copy of METADATA in RAM

     MB85RS* fram;
};

#endif /* SOFTWAREUPDATESERVICE_H_ */
