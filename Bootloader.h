/*
 * Bootloader.h
 *
 *  Created on: 22 Jan 2020
 *      Author: Casper
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "MB85RS.h"
#include "FRAMMap.h"

#define BOOT_PERMANENT_FLAG          0x80

class Bootloader{
private:
    MB85RS *fram;
public:
    uint8_t current_slot;
    Bootloader(MB85RS &fram);
    void JumpSlot();
    static unsigned char getCurrentSlot();
};



#endif /* BOOTLOADER_H_ */
