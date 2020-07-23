/*
 * FRAMMap.h
 *
 *  Created on: Mar 2, 2020
 *      Author: casper
 */

#ifndef FRAMMAP_H_
#define FRAMMAP_H_

#define FRAM_BOOT_SUCCES_FLAG                   0
#define FRAM_RESET_COUNTER                      1  //size: 3 bytes (slot0, slot1, slot2)
#define FRAM_RESET_CAUSE                        4  //size: 3 bytes (17 bits to be precise)
#define FRAM_TOTAL_UPTIME                       7
#define FRAM_BOOTLOADER_TARGET                  11

#define FRAM_METADATA_SLOT1_STATE               12
#define FRAM_METADATA_SLOT1_MD5                 13
#define FRAM_METADATA_SLOT1_VERSION             29
#define FRAM_METADATA_SLOT1_NR_OF_BLOCKS        37
#define FRAM_METADATA_SLOT1_CRCS                41
#define FRAM_METADATA_SLOT1_CRC_CHECKLIST       16425
#define FRAM_METADATA_SLOT1_BLOCK_CHECKLIST     18473

#define FRAM_METADATA_SLOT2_STATE               20521
#define FRAM_METADATA_SLOT2_MD5                 20522
#define FRAM_METADATA_SLOT2_VERSION             20538
#define FRAM_METADATA_SLOT2_NR_OF_BLOCKS        20546
#define FRAM_METADATA_SLOT2_CRCS                20550
#define FRAM_METADATA_SLOT2_CRC_CHECKLIST       36934
#define FRAM_METADATA_SLOT2_BLOCK_CHECKLIST     38982

#define SOFTWAREUPDATE_SERVICE_FRAM_OFFSET      12

#define FRAM_DEVICE_SPECIFIC_SPACE              41030
//      SOFTWAREUPDATE_SERVICE_FRAM_SIZE 4663

#endif /* FRAMMAP_H_ */
