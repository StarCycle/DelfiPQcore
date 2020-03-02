/*
 * FRAMMap.h
 *
 *  Created on: Mar 2, 2020
 *      Author: casper
 */

#ifndef FRAMMAP_H_
#define FRAMMAP_H_

#define FRAM_BOOT_SUCCES_FLAG   0
#define FRAM_RESET_COUNTER      1
#define FRAM_RESET_CAUSE        2  //size: 3 bytes (17 bits to be precise)

#define BOOTLOADER_TARGET_REG               5

#define SOFTWAREUPDATE_SERVICE_FRAM_OFFSET  6
//      SOFTWAREUPDATE_SERVICE_FRAM_SIZE 4663

#endif /* FRAMMAP_H_ */
