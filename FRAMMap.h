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
#define FRAM_METADATA_SLOT1_CRCS                39
#define FRAM_METADATA_SLOT1_CRC_CHECKLIST       2087
#define FRAM_METADATA_SLOT1_BLOCK_CHECKLIST     2343

#define FRAM_METADATA_SLOT2_STATE               2599
#define FRAM_METADATA_SLOT2_MD5                 2600
#define FRAM_METADATA_SLOT2_VERSION             2616
#define FRAM_METADATA_SLOT2_NR_OF_BLOCKS        2624
#define FRAM_METADATA_SLOT2_CRCS                2626
#define FRAM_METADATA_SLOT2_CRC_CHECKLIST       4674
#define FRAM_METADATA_SLOT2_BLOCK_CHECKLIST     4930

#define SOFTWAREUPDATE_SERVICE_FRAM_OFFSET      12

#define FRAM_DEVICE_SPECIFIC_SPACE              5186
//      SOFTWAREUPDATE_SERVICE_FRAM_SIZE 4663

#endif /* FRAMMAP_H_ */
