/*
 * Bootloader.h
 *
 *  Created on: 22 Jan 2020
 *      Author: Casper
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "MB85RS.h"

class Bootloader{
private:
    MB85RS *fram;
public:
    Bootloader(MB85RS &fram);
    void JumpSlot();
};



#endif /* BOOTLOADER_H_ */
