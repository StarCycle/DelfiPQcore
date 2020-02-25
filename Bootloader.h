/*
 * Bootloader.h
 *
 *  Created on: 22 Jan 2020
 *      Author: Casper
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "MB85RS.h"

#define BOOT_PERMANENTLY   0x80
#define FRAM_TARGET_SLOT   0x7FF0

class Bootloader{
private:
    MB85RS *fram;
public:
    uint8_t current_slot;
    Bootloader(MB85RS &fram);
    void JumpSlot();
};



#endif /* BOOTLOADER_H_ */
