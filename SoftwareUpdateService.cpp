/*
 * SoftwareUpdateService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Jasper Haenen
 */

#include "SoftwareUpdateService.h"

uint8_t HexStringToNibble(char c){
    if ((c) >= 'a'){
        return c - 87;
    }else if ((c) >= 'A'){
        return c - 55;
    }else{
        return c - 48;
    }
}

uint8_t NibblesToByte(uint8_t msb, uint8_t lsb){
    return ((msb << 4) | lsb);
}

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   DataBus &interface       Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */

SoftwareUpdateService::SoftwareUpdateService(MB85RS &fram_in, uint8_t * versionString) {
    fram = &fram_in;
    hasVersionNumber = true;
    this->versionNumber[0] = NibblesToByte(HexStringToNibble(versionString[0]),HexStringToNibble(versionString[1]));
    this->versionNumber[1] = NibblesToByte(HexStringToNibble(versionString[2]),HexStringToNibble(versionString[3]));
    this->versionNumber[2] = NibblesToByte(HexStringToNibble(versionString[4]),HexStringToNibble(versionString[5]));
    this->versionNumber[3] = NibblesToByte(HexStringToNibble(versionString[6]),HexStringToNibble(versionString[7]));
    this->versionNumber[4] = NibblesToByte(HexStringToNibble(versionString[8]),HexStringToNibble(versionString[9]));
    this->versionNumber[5] = NibblesToByte(HexStringToNibble(versionString[10]),HexStringToNibble(versionString[11]));
    this->versionNumber[6] = NibblesToByte(HexStringToNibble(versionString[12]),HexStringToNibble(versionString[13]));
    this->versionNumber[7] = NibblesToByte(HexStringToNibble(versionString[14]),HexStringToNibble(versionString[15]));

}

SoftwareUpdateService::SoftwareUpdateService(MB85RS &fram_in) {
    fram = &fram_in;
    hasVersionNumber = false;
    Console::log("WARNING: no SW Version!");
}

bool SoftwareUpdateService::process(DataMessage &command, DataMessage &workingBuffer) {
    if (command.getService() == SOFTWAREUPDATE_SERVICE) {
        // prepare response frame
//        workingBuffer.setDestination(command.getSource());
//        workingBuffer.setSource(interface.getAddress());
        workingBuffer.setService(SOFTWAREUPDATE_SERVICE);
        workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);

        payload_data = workingBuffer.getDataPayload();

        if(command.getDataPayload()[COMMAND_METHOD] != ERASE_SLOT) state_flags &= ~ERASE_FLAG; //unset the Erase flag, if the command is not to erase.

        switch (command.getDataPayload()[COMMAND_METHOD]) {

        case START_OTA:
            if((command.getPayloadSize() == 2) || (command.getPayloadSize() == 3)){
                if(command.getDataPayload()[COMMAND_DATA] == 1 || command.getDataPayload()[COMMAND_DATA] == 2) {
                    if(command.getPayloadSize() == 2){
                        startOTA(command.getDataPayload()[COMMAND_DATA], command.getDataPayload()[COMMAND_DATA+1] == 1);
                    }else{
                        startOTA(command.getDataPayload()[COMMAND_DATA], false);
                    }

                    if(payload_data[0] == NO_ERROR) Console::log("OTA started!");

                    } else throw_error(SLOT_OUT_OF_RANGE);
                } else throw_error(PARAMETER_MISMATCH);
                break;

        case SET_METADATA:
            if(command.getPayloadSize() == METADATA_SIZE) {
                setMetadata(&(command.getDataPayload()[COMMAND_DATA]));
                if(payload_data[0] == NO_ERROR) Console::log("Metadata received!");

                } else throw_error(PARAMETER_MISMATCH);
                break;

        case GET_METADATA:
            if(command.getPayloadSize() == 2) {
                if(command.getDataPayload()[COMMAND_DATA] == 1 || command.getDataPayload()[COMMAND_DATA] == 2) {
                    getMetadata(command.getDataPayload()[COMMAND_DATA] - 1);
                    if(payload_data[0] == NO_ERROR) {
                        print_metadata(&payload_data[COMMAND_DATA]);
                        Console::log("Metadata sent!");
                    }
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SET_PARTIAL_CRCS:
            if(command.getPayloadSize() <= BLOCK_SIZE + 2 + 1) { //2 extra bytes for offset bytes
//                serial.print("SIZE BYTE msB: ");
//                serial.println(command.getPayload()[COMMAND_DATA + 1],DEC);
//                serial.print("SIZE BYTE lsB: ");
//                serial.println(command.getPayload()[COMMAND_DATA],DEC);
//                serial.print("CRC OFFSET: ");
//                serial.println(command.getPayload()[COMMAND_DATA] | (command.getPayload()[COMMAND_DATA + 1] << 8), DEC);
                setPartialCRCs(&(command.getDataPayload()[COMMAND_DATA+2]), command.getPayloadSize() - 3, command.getDataPayload()[COMMAND_DATA] | (command.getDataPayload()[COMMAND_DATA + 1] << 8));
                if(payload_data[0] == NO_ERROR) Console::log("CRC!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SET_BLOCK:
            if(command.getPayloadSize() <= BLOCK_SIZE + 2 + 1) {
                setBlock(&(command.getDataPayload()[COMMAND_DATA + 2]), command.getDataPayload()[COMMAND_DATA] | (command.getDataPayload()[COMMAND_DATA + 1] << 8));
                if(payload_data[0] == NO_ERROR) Console::log("BLOCK!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case CHECK_MD5:
            if(command.getPayloadSize() == 1 + 1) {
                if(command.getDataPayload()[COMMAND_DATA] == 1 || command.getDataPayload()[COMMAND_DATA] == 2) {
                    checkMD5(command.getDataPayload()[COMMAND_DATA]);
                    if(payload_data[0] == NO_ERROR) Console::log("MD5 is correct!");
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case STOP_OTA:
            if(command.getPayloadSize() == 1) {
                stopOTA();
                if(payload_data[0] == NO_ERROR) Console::log("OTA is stopped!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case ERASE_SLOT:
            Console::log("DataPayloadSize: %d", command.getPayloadSize());
            if(command.getPayloadSize() == 1 + 1) {
                if((state_flags & ERASE_FLAG) == 0) {
                    if(command.getDataPayload()[COMMAND_DATA] == 1 || command.getDataPayload()[COMMAND_DATA] == 2) {
                        slot_erase = command.getDataPayload()[COMMAND_DATA];
                        state_flags |= ERASE_FLAG;
                        Console::log("Are you sure(13)?");
                    } else throw_error(SLOT_OUT_OF_RANGE);
                } else {
                    if(command.getDataPayload()[COMMAND_DATA] == ACKNOWLEDGE) {
                        eraseSlot(slot_erase);
                        if(payload_data[0] == NO_ERROR) Console::log("Slot is erased!");
                    } else throw_error(PARAMETER_MISMATCH);
                }
            } else throw_error(PARAMETER_MISMATCH);
            break;
        case SET_BOOT_SLOT:
            if(command.getPayloadSize() == 2 + 1) {
                if(command.getDataPayload()[COMMAND_DATA] < 3) {
                    setBootSlot(command.getDataPayload()[COMMAND_DATA], command.getDataPayload()[COMMAND_DATA + 1]);
                    if(payload_data[0] == NO_ERROR) Console::log("Slot code executed successfully!");
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;
        case GET_MISSED_BLOCKS:
            if(command.getPayloadSize() == 1) {
                getMissedBlocks();
            } else throw_error(PARAMETER_MISMATCH);
            break;
        case GET_MISSED_CRC:
            if(command.getPayloadSize() == 1) {
                getMissedCRCs();
            } else throw_error(PARAMETER_MISMATCH);
            break;
        case GET_VERSION_NUMBER:
            if(command.getPayloadSize() == 1) {
                getVersionNumber();
            } else throw_error(PARAMETER_MISMATCH);
            break;
        default:
            break;
        }

        workingBuffer.setPayloadSize(payload_size);

            // command processed
            return true;
        } else {
            // this command is related to another service,
            // report the command was not processed
            return false;
        }
}

void SoftwareUpdateService::getVersionNumber(){
    Console::log("SoftwareVersionService: Software Version Request");
    // respond to ping
    payload_size = 1;
    payload_data[0] = Bootloader::getCurrentSlot();
    if(this->hasVersionNumber == true){
        payload_size = 9;
        Console::log("Has SW Version: %x%x%x%x%x%x%x%x", versionNumber[0],versionNumber[1],versionNumber[2],versionNumber[3],versionNumber[4],versionNumber[5],versionNumber[6],versionNumber[7]);
        payload_data[1] = this->versionNumber[0];
        payload_data[2] = this->versionNumber[1];
        payload_data[3] = this->versionNumber[2];
        payload_data[4] = this->versionNumber[3];
        payload_data[8] = this->versionNumber[4];
        payload_data[6] = this->versionNumber[5];
        payload_data[7] = this->versionNumber[6];
        payload_data[8] = this->versionNumber[7];
    }
}

void SoftwareUpdateService::startOTA(unsigned char slot_number, bool allow_resume) {
    if(Bootloader::getCurrentSlot() != (slot_number)) { //Warning! Do not reprogram the current program, very bad idea

        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        fram->read(UPDATE_PROGRESS_STATE, &state_flags, 1);

        if(((state_flags & UPDATE_FLAG) == 0) && !allow_resume){ //system is not already in update mode.
            unsigned char slotState;
            if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
            if(slot_number == 1){
                fram->read(SLOT1_METADATA_STATE, &slotState, 1);
            }else if(slot_number == 2){
                fram->read(SLOT2_METADATA_STATE, &slotState, 1);
            }
            Console::log("Slot State: 0x%x", (int) slotState);
            if((slotState & 0x03) == EMPTY) {
                Console::log("Slot is empty!");
                slotState = PARTIAL; //Set status to Partial OTA
                slotState |= UPDATE_FLAG;
                if(slot_number == 2){
                    slotState |= SLOT_SELECT_FLAG;
                }
                slotState &= ~(METADATA_FLAG | PARTIAL_CRC_FLAG | MD5_CORRECT_FLAG); //unset these flags, just to be sure

                update_slot = slot_number; //store my updateSlot in RAM
                state_flags = slotState; //store my update progress also in RAM

                //write my current update Status to FRAM
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);
                if(slot_number == 1){
                    fram->write(SLOT1_METADATA_STATE, &state_flags, 1);
                }else if(slot_number == 2){
                    fram->write(SLOT2_METADATA_STATE, &state_flags, 1);
                }

                //initialize the checklists with zeros
                for(int i = 0; i < (MAX_BLOCK_AMOUNT/BYTE_SIZE)+1; i++) crc_received[i] = 0;
                for(int i = 0; i < (MAX_BLOCK_AMOUNT/BYTE_SIZE)+1; i++) blocks_received[i] = 0;
                //write the FRAM Progress checklists
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                fram->write(UPDATE_PROGRESS_CRC, crc_received, UPDATE_PROGRESS_CHECKLIST_SIZE);
                fram->write(UPDATE_PROGRESS_BLOCKS, blocks_received, UPDATE_PROGRESS_CHECKLIST_SIZE);

                //write the Cmd Reply:
                payload_size = 1;
                payload_data[0] = NO_ERROR;

            } else if((slotState & 0x03) == PARTIAL) { //slot is not empty but partially updated
                // first check if the partially updated slot Aligns with current progress and check for resume:
                Console::log("Update still in progress, progress state: 0x%x", (int) state_flags );
                Console::log("  -  slot state: %x", (int) slotState);
                if((slotState == state_flags) && ((state_flags & UPDATE_FLAG) == UPDATE_FLAG )){ //check if we can resume, due to the progress status being identical to the slot status:
                    if(allow_resume){ //if we allow resume
                        update_slot = slot_number; //store my updateSlot in RAM

                        //retrieve number of blocks from MetaData
                        unsigned char nrOfBlocksBuf[METADATA_NR_OF_BLOCKS_SIZE] = {0};
                        if(slot_number == 1){
                            fram->read(SLOT1_METADATA_NR_OF_BLOCKS, nrOfBlocksBuf, METADATA_NR_OF_BLOCKS_SIZE);
                        }else if(slot_number == 2){
                            fram->read(SLOT2_METADATA_NR_OF_BLOCKS, nrOfBlocksBuf, METADATA_NR_OF_BLOCKS_SIZE);
                        }

                        //get nr of blocks
                        num_update_blocks = nrOfBlocksBuf[0] | (nrOfBlocksBuf[1] << 8);

                        //read the FRAM Progress checklists
                        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                        fram->read(UPDATE_PROGRESS_CRC, crc_received, UPDATE_PROGRESS_CHECKLIST_SIZE);
                        fram->read(UPDATE_PROGRESS_BLOCKS, blocks_received, UPDATE_PROGRESS_CHECKLIST_SIZE);

                        //write my current update Status to FRAM
                        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                        fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);
                        if(slot_number == 1){
                            fram->write(SLOT1_METADATA_STATE, &state_flags, 1);
                        }else if(slot_number == 2){
                            fram->write(SLOT2_METADATA_STATE, &state_flags, 1);
                        }
                        Console::log("Update Resumed!!");

                        //write Command Reply:
                        payload_size = 1;
                        payload_data[0] = NO_ERROR;

                    }else{ // decided not to resume but slot is currently updating
                        return throw_error(UPDATE_ALREADY_STARTED);
                    }
                }else{ // either impossible to resume since its a different slot, or not in-update on non-empty slot
                    return throw_error(SLOT_NOT_EMPTY);
                }
            } else return throw_error(SLOT_NOT_EMPTY);
        } else return throw_error(UPDATE_ALREADY_STARTED);
    } else return throw_error(SELF_ACTION);
}

void SoftwareUpdateService::setMetadata(unsigned char* metadata) {

    if((state_flags & UPDATE_FLAG) == UPDATE_FLAG) { //if state is in-Update
        if((state_flags & METADATA_FLAG) == 0) { //if METADATA has not yet been received
            unsigned short temp_num_blocks = metadata[NUM_BLOCKS_OFFSET - 1] | (metadata[NUM_BLOCKS_OFFSET] << 8);
            if(temp_num_blocks <= MAX_BLOCK_AMOUNT) { //check if the metadata block amount is 'correct'

                //set METADATA in FRAM except for the state byte
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                if(update_slot == 1){
                    fram->write(SLOT1_METADATA_MD5, metadata, METADATA_SIZE - 1);
                }else if(update_slot == 2){
                    fram->write(SLOT2_METADATA_MD5, metadata, METADATA_SIZE - 1);
                }

                //update stateByte and NR of Blocks
                num_update_blocks = temp_num_blocks;
                state_flags |= METADATA_FLAG;   //set MetaData received Flag
                //write updates of flag to FRAM
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);
                if(update_slot == 1){
                    fram->write(SLOT1_METADATA_STATE, &state_flags, 1);
                }else if(update_slot == 2){
                    fram->write(SLOT2_METADATA_STATE, &state_flags, 1);
                }
                Console::log("METADATA RECEIVED, Status: 0x%x", (int)state_flags);

                //write the Cmd Reply:
                payload_size = 1;
                payload_data[0] = NO_ERROR;

            } else return throw_error(UPDATE_TO_BIG);
        } else return throw_error(METADATA_ALREADY_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::getMetadata(unsigned char slot_number) {
    payload_size = 1 + METADATA_SIZE ;
    payload_data[0] = NO_ERROR;

    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    if(slot_number == 1){
        fram->read(SLOT1_METADATA, &payload_data[1], METADATA_SIZE);
    }else if(slot_number == 2){
        fram->read(SLOT2_METADATA, &payload_data[1], METADATA_SIZE);
    }
}

void SoftwareUpdateService::setPartialCRCs(unsigned char* crc_block, unsigned char num_bytes, unsigned short crc_offset) {
//    serial.println(crc_offset, DEC);
    if((state_flags & UPDATE_FLAG) == UPDATE_FLAG) { //update is active
        if((state_flags & METADATA_FLAG) ==  METADATA_FLAG) { //metaData has been received
            if(crc_offset + num_bytes <= num_update_blocks) { //the amount of CRCs received do not exceed the number of expected CRCs

                //write CRCs to FRAM
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                if(update_slot == 1){
                    fram->write(SLOT1_PAR_CRC + crc_offset, crc_block, num_bytes);
                }else if(update_slot == 2){
                    fram->write(SLOT2_PAR_CRC + crc_offset, crc_block, num_bytes);
                }

//                serial.print("Writing CRCs: ");
//                serial.println(num_bytes, DEC);
                //update checklist in RAM
                for(int k = 0; k < num_bytes; k++){
                    crc_received[(crc_offset+k) / BYTE_SIZE] |= 1 << ((crc_offset+k) % BYTE_SIZE);
                }

                //write checklist changes to FRAM
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
//                serial.print("Writing CRC CheckList Bytes: ");
//                serial.println(((num_bytes-1)/BYTE_SIZE) + 1, DEC);
//                for(int i = 0; i < (((num_bytes-1)/BYTE_SIZE) + 1); i++){
//                    serial.println(crc_received[(crc_offset) / BYTE_SIZE + i], DEC);
//                }
                fram->write(UPDATE_PROGRESS_CRC + (crc_offset / BYTE_SIZE), &crc_received[(crc_offset) / BYTE_SIZE], ((num_bytes-1)/BYTE_SIZE) + 1);

                //Check if all CRCs are received
                if(this->getNumOfMissedCRCs() == 0){
                    Console::log("ALL CRCS RECEIVED!!");
                    //set Flag
                    state_flags |= PARTIAL_CRC_FLAG;
                    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                    fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);
                    if(update_slot == 1){
                        fram->write(SLOT1_METADATA_STATE, &state_flags, 1);
                    }else if(update_slot == 2){
                        fram->write(SLOT2_METADATA_STATE, &state_flags, 1);
                    }
                }

                //set Response
                payload_size = 1;
                payload_data[0] = NO_ERROR;

            } else return throw_error(PARAMETER_MISMATCH);
        } else return throw_error(METADATA_NOT_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::setBlock(unsigned char* data_block, uint16_t block_offset) {

    if((state_flags & UPDATE_FLAG) == UPDATE_FLAG) { //update is started
        if((state_flags & METADATA_FLAG) == METADATA_FLAG) { //metaData has been received
            if((state_flags & PARTIAL_CRC_FLAG) == PARTIAL_CRC_FLAG) { //all CRCs are received
                if(block_offset <= num_update_blocks) { //if received block is within expected range

                    //check if CRC matches
                    if(checkPartialCRC(data_block, block_offset)) {
                        //write block to FLASH
                        unsigned int memloc;
                        unsigned int sector =  1 << (((block_offset * BLOCK_SIZE + (update_slot - 1) * SLOT_SIZE)) / SECTOR_SIZE);

                        //convert sector back to memory location
                        // memloc = offset_bank1 + sector * sector_size
                        memloc = (BANK1_ADDRESS + (update_slot - 1) * SLOT_SIZE + block_offset * BLOCK_SIZE);
                        // this looks like a weird construction, however memory is appearantly still sector based:
                        // http://dev.ti.com/tirex/explore/node?node=ACjtNKxGz96GtFvTawWu-g__z-lQYNj__LATEST
                        if(!MAP_FlashCtl_A_unprotectMemory(memloc, memloc + BLOCK_SIZE - 1)) return throw_error(NO_SLOT_ACCESS);
                        if(!MAP_FlashCtl_A_programMemory(data_block, (void*) memloc, BLOCK_SIZE)) return throw_error(NO_SLOT_ACCESS);
                        if(!MAP_FlashCtl_A_protectMemory(memloc, memloc + BLOCK_SIZE - 1)) return throw_error(NO_SLOT_ACCESS);

                        //update checklist in RAM
                        blocks_received[block_offset / BYTE_SIZE] |= (1 << (block_offset) % BYTE_SIZE);

                        //write checklist changes to FRAM
                        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                        fram->write(UPDATE_PROGRESS_BLOCKS + (block_offset / BYTE_SIZE), &blocks_received[block_offset / BYTE_SIZE], 1);

                        //Check if all Blocks are received
                        if(this->getNumOfMissedBlocks() == 0){
                            Console::log("ALL BLOCKS RECEIVED!!");
                            //set Flag
                            state_flags &= ~(0x03);
                            state_flags |= FULL;
                            if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                            fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);
                            if(update_slot == 1){
                                fram->write(SLOT1_METADATA_STATE, &state_flags, 1);
                            }else if(update_slot == 2){
                                fram->write(SLOT2_METADATA_STATE, &state_flags, 1);
                            }
                        }

                        //set Response
                        payload_size = 1;
                        payload_data[0] = NO_ERROR;

                    } else {
                        return throw_error(CRC_MISMATCH);
                    }
                } else return throw_error(OFFSET_OUT_OF_RANGE);
            } else return throw_error(PARTIAL_NOT_RECEIVED);
        } else return throw_error(METADATA_NOT_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

bool SoftwareUpdateService::checkPartialCRC(unsigned char* data_block, uint16_t block_offset) {
    unsigned char val = 0;

    for(int i = 0; i < BLOCK_SIZE; i++) {
        val = CRC_TABLE[val ^ data_block[i]];
    }

    unsigned char crc;
    if(!fram->ping()) {
        throw_error(NO_FRAM_ACCESS);
        return false;
    }
    //read CRC
    if(update_slot == 1){
        fram->read(SLOT1_PAR_CRC + block_offset, &crc, 1);
    }else if(update_slot == 2){
        fram->read(SLOT2_PAR_CRC + block_offset, &crc, 1);
    }


    if(crc != val) {
        Console::log("Block offset: %d", (int) block_offset);
        Console::log("Calculated CRC: %d", (int) val);
        Console::log("Stored CRC: %d", (int) crc);
    }

    return crc == val;
}

void SoftwareUpdateService::checkMD5(unsigned char slot_number) {
    unsigned char status = 0;

    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    if(slot_number == 1){
        fram->read(SLOT1_METADATA_STATE, &status, 1);
    }else if(slot_number == 2){
        fram->read(SLOT2_METADATA_STATE, &status, 1);
    }

    if((status & FULL) == FULL){
        unsigned char digest[MD5_SIZE];

        MD5_CTX md5_c;
        MD5_Init(&md5_c);

        uint16_t num_blocks;
    //    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    //    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (unsigned char*)&num_blocks, sizeof(uint16_t));
        unsigned char nrOfBlocksBuf[METADATA_NR_OF_BLOCKS_SIZE] = {0};
        if(slot_number == 1){
            fram->read(SLOT1_METADATA_NR_OF_BLOCKS, nrOfBlocksBuf, METADATA_NR_OF_BLOCKS_SIZE);
        }else if(slot_number == 2){
            fram->read(SLOT2_METADATA_NR_OF_BLOCKS, nrOfBlocksBuf, METADATA_NR_OF_BLOCKS_SIZE);
        }

        //get nr of blocks
        num_blocks = nrOfBlocksBuf[0] | (nrOfBlocksBuf[1] << 8);

        unsigned char meta_crc[MD5_SIZE];
        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        if(slot_number == 1){
            fram->read(SLOT1_METADATA_MD5, meta_crc, MD5_SIZE);
        }else if(slot_number == 2){
            fram->read(SLOT2_METADATA_MD5, meta_crc, MD5_SIZE);
        }

        MD5_Update(&md5_c, (unsigned char*)(BANK1_ADDRESS + (slot_number - 1) * SLOT_SIZE), num_blocks * BLOCK_SIZE);

        MD5_Final(digest, &md5_c);

        bool equal = true;
        for(int i = 0; i < MD5_SIZE; i++) {
            if(digest[i] != meta_crc[i]) {
                equal = false;
                Console::log("MD5 InCorrect! 0x%x, expected 0x%x", digest[i], meta_crc[i]);
            }
        }

        if(equal){
            status |= MD5_CORRECT_FLAG;
            state_flags |= MD5_CORRECT_FLAG;
            Console::log("MD5 Correct! Status: 0x%x", (int) status);
        }
        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        if(slot_number == 1){
            fram->write(SLOT1_METADATA_STATE, &status, 1);
        }else if(slot_number == 2){
            fram->write(SLOT2_METADATA_STATE, &status, 1);
        }

        //set Response
        payload_size = 1;
        payload_data[0] = NO_ERROR;
        payload_data[1] = equal;

    } else return throw_error(SLOT_NOT_PROGRAMMED);
}

void SoftwareUpdateService::stopOTA() {
    Console::log("stopping update, status: 0x%x", (int) state_flags);
    //check MD5 before killing
    if((state_flags & FULL) == FULL) {
        checkMD5(update_slot);
    }

    if((state_flags & UPDATE_FLAG) == UPDATE_FLAG) { //OTA is running, time to kill it.
        state_flags &= ~UPDATE_FLAG;

        Console::log("stopped update, status: 0x%x", (int) state_flags);
        //Write State
        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        if(update_slot == 1){
            fram->write(SLOT1_METADATA_STATE, &state_flags, 1);
        }else if(update_slot == 2){
            fram->write(SLOT2_METADATA_STATE, &state_flags, 1);
        }

        state_flags = 0; //destroy progress flags
        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);

        //set Response
        payload_size = 1;
        payload_data[0] = NO_ERROR;

    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::eraseSlot(unsigned char slot) {
    if(Bootloader::getCurrentSlot() != slot ) { //dont erase yourself, very bad idea
        if((state_flags & UPDATE_FLAG) == 0) { //check if update is not in progress
            //delete MetaData
           unsigned char empty[METADATA_SIZE] = { 0 };
           if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
           if(slot == 1){
               fram->write(SLOT1_METADATA, empty, METADATA_SIZE);
           }else if(slot == 2){
               fram->write(SLOT2_METADATA, empty, METADATA_SIZE);
           }

           unsigned int memloc;
           if(slot == 1){
               memloc = 0x100000; //slot 1 start
           }else if(slot == 2){
               memloc = 0x180000; //slot 2 start
           }

           // http://dev.ti.com/tirex/explore/node?node=ACjtNKxGz96GtFvTawWu-g__z-lQYNj__LATEST
           if(!MAP_FlashCtl_A_unprotectMemory(memloc, memloc + SLOT_SIZE - 1)) return throw_error(NO_SLOT_ACCESS);
           if(!MAP_FlashCtl_A_performMassErase()) return throw_error(NO_SLOT_ACCESS);
           if(!MAP_FlashCtl_A_protectMemory(memloc, memloc + SLOT_SIZE - 1)) return throw_error(NO_SLOT_ACCESS);

           state_flags = 0; //destroy progress flag
           if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
           fram->write(UPDATE_PROGRESS_STATE, &state_flags, 1);

           //set Response
           payload_size = 1;
           payload_data[0] = NO_ERROR;

        } else return throw_error(UPDATE_ALREADY_STARTED);
    } else return throw_error(SELF_ACTION);
}

void SoftwareUpdateService::setBootSlot(unsigned char slot, bool permanent) {
    if(slot == 0) { //if setting SLOT0 (fallback slot), no worries, just do it.
        uint8_t target_slot = (permanent) ? BOOT_PERMANENT_FLAG : 0;
        fram->write(BOOTLOADER_TARGET_REG, &target_slot, 1);
        //set Response
        payload_size = 1;
        payload_data[0] = NO_ERROR;

    } else {
        unsigned char slotFlag = 0;
        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        if(slot == 1){
            fram->read(SLOT1_METADATA_STATE, &slotFlag, 1);
        }else if(slot == 2){
            fram->read(SLOT2_METADATA_STATE, &slotFlag, 1);
        }
        Console::log("Status flag of Target: 0x%x", (int) slotFlag);

        //checkMD5(slot); No reason to check again, flag should be set already.

        if((slotFlag & MD5_CORRECT_FLAG) == 0) return throw_error(MD5_MISMATCH);
        if((slotFlag & FULL) == FULL) {
            uint8_t target_slot = slot | ((permanent) ? BOOT_PERMANENT_FLAG : 0);
            fram->write(BOOTLOADER_TARGET_REG, &target_slot, 1);
            //set Response
            payload_size = 1;
            payload_data[0] = NO_ERROR;

#if defined (__MSP432P401R__)
            this->setPostFunc([](){ MAP_SysCtl_rebootDevice();});
#elif defined (__MSP432P4111__)
            this->setPostFunc([](){ MAP_SysCtl_A_rebootDevice();});
#endif
        } else return throw_error(SLOT_NOT_PROGRAMMED);
    }
}

unsigned int SoftwareUpdateService::getNumOfMissedBlocks() {
    unsigned int count = 0;
    for(int missed_pointer = 0; missed_pointer < num_update_blocks; missed_pointer++) {
        if((blocks_received[missed_pointer / BYTE_SIZE] & (1 << missed_pointer % BYTE_SIZE)) == 0) { //there is a block missing here
           count++;
        }
    }
    return count;
}

unsigned int SoftwareUpdateService::getNumOfMissedCRCs() {
    unsigned int count = 0;
    for(int missed_pointer = 0; missed_pointer < num_update_blocks; missed_pointer++) {
        if((crc_received[missed_pointer / BYTE_SIZE] & (1 << missed_pointer % BYTE_SIZE)) == 0) { //there is a block missing here
           count++;
        }
    }
    return count;
}

void SoftwareUpdateService::getMissedBlocks() {
    if((state_flags & UPDATE_FLAG) == UPDATE_FLAG) {

        int missed_pointer = 0;
        //set Response
        payload_size = 1;
        payload_data[0] = NO_ERROR;

        switch(state_flags) {
            case FULL:
                Console::log("The requested slot is already fully programmed.");
                return;
            case EMPTY:
                return throw_error(UPDATE_NOT_STARTED);
            default:
                break;
        }

        //get up to 32 missed blocks
        while(payload_size < (2*32+1)){

            if(missed_pointer == num_update_blocks) { //if everything is checked, just return
                missed_pointer = 0;
                return;
            }

            if((blocks_received[missed_pointer / BYTE_SIZE] & (1 << missed_pointer % BYTE_SIZE)) == 0) { //there is a block missing here
                memcpy(&payload_data[COMMAND_DATA + (payload_size - 2)], &missed_pointer, sizeof(uint16_t));
                payload_size += 2;
                Console::log("Block %d is missing", (int) missed_pointer);
            }

            missed_pointer++;
        }
    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::getMissedCRCs() {
    if((state_flags & UPDATE_FLAG) == UPDATE_FLAG) {

        int missed_pointer = 0;
        //set Response
        payload_size = 1;
        payload_data[0] = NO_ERROR;

        switch(state_flags) {
            case FULL:
                Console::log("The requested slot is already fully programmed.");
                return;
            case EMPTY:
                return throw_error(UPDATE_NOT_STARTED);
            default:
                break;
        }

        //get up to 32 missed blocks
        while(payload_size < (2*32+1)){

            if(missed_pointer == num_update_blocks) { //if everything is checked, just return
                missed_pointer = 0;
                return;
            }

            if((crc_received[missed_pointer / BYTE_SIZE] & (1 << missed_pointer % BYTE_SIZE)) == 0) { //there is a block missing here
                memcpy(&payload_data[COMMAND_DATA + (payload_size - 2)], &missed_pointer, sizeof(uint16_t));
                payload_size += 2;
                Console::log("CRC %d is missing.", (int) missed_pointer);
            }

            missed_pointer++;
        }
    } else return throw_error(UPDATE_NOT_STARTED);
}


void SoftwareUpdateService::print_metadata(unsigned char* metadata) {
    Console::log("Metadata: %x", (int) *metadata);
    Console::log("\tSlot status: ");
    switch (metadata[0])
    {
        case EMPTY:
            Console::log("Emtpy");
            break;
        case PARTIAL:
            Console::log("Partial");
            break;
        case FULL:
            Console::log("Full");
            break;
        default:
            Console::log("Unknown slot status!");
            break;
    }
    Console::log("\tVersion: %x%x%x%x%x%x%x%x",metadata[MD5_SIZE+1],metadata[MD5_SIZE+2],metadata[MD5_SIZE+3],metadata[MD5_SIZE+4],metadata[MD5_SIZE+5],metadata[MD5_SIZE+6],metadata[MD5_SIZE+7],metadata[MD5_SIZE+8]);
    Console::log("\tNumber of blocks: %d", (int) (metadata[MD5_SIZE+10] << 8 | metadata[MD5_SIZE+9]));
    Console::log("\tMD5: %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
                 metadata[0], metadata[1],  metadata[2], metadata[3],
                 metadata[4], metadata[5],  metadata[6], metadata[7],
                 metadata[8], metadata[9],  metadata[10], metadata[11],
                 metadata[12], metadata[13],  metadata[14], metadata[15]);
}

void SoftwareUpdateService::throw_error(unsigned char error) {
    Console::log("Error(%d)", (int) error);

    switch (error)
    {
    case NO_FRAM_ACCESS:
        Console::log("No access to FRAM.");
        break;
    case NO_SLOT_ACCESS:
        Console::log("No access to requested slot.");
        break;
    case SLOT_OUT_OF_RANGE:
        Console::log("Requested slot is out of range.");
        break;
    case MEMORY_FULL:
        Console::log("The memory is full.");
        break;
    case PARAMETER_MISMATCH:
        Console::log("Invalid parameter size provided to function.");
        break;
    case UPDATE_NOT_STARTED:
        Console::log("The update has not started yet.");
        break;
    case UPDATE_ALREADY_STARTED:
        Console::log("The update is still in progress.");
        break;
    case METADATA_ALREADY_RECEIVED:
        Console::log("The metadata has already been received.");
        break;
    case METADATA_NOT_RECEIVED:
        Console::log("The metadata has not been received yet.");
        break;
    case PARTIAL_ALREADY_RECEIVED:
        Console::log("The partial crcs have already been received.");
        break;
    case PARTIAL_NOT_RECEIVED:
        Console::log("The partial crcs have not been received yet.");
        break;
    case CRC_MISMATCH:
        Console::log("A partial crc mismatch has occurred.");
        break;
    case MD5_MISMATCH:
        Console::log("The md5 hash does not match.");
        break;
    case OFFSET_OUT_OF_RANGE:
        Console::log("The requested offset is out of range.");
        break;
    case SLOT_NOT_EMPTY:
        Console::log("The requested slot is not empty yet.");
        break;
    case UPDATE_TO_BIG:
        Console::log("The update is too big for the memory slot.");
        break;
    case SLOT_NOT_PROGRAMMED:
        Console::log("The requested slot is not (completely) programmed.");
        break;
    case UPDATE_NOT_CURRENT_SESSION:
        Console::log("The update cannot start, because the update is not from the current session. Erase the slot first and retry.");
        break;
    case SELF_ACTION:
        Console::log("The requested slot  cannot perform an action on itself.");
        break;
    case NO_VERSION_NUMBER:
        Console::log("Current Software does not have a version number!");
        break;
    default:
        break;
    }

    payload_size = 1;
    payload_data[0] = NO_ERROR;
}
