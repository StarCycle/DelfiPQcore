/*
 * Bootloader.cpp
 *
 *  Created on: 22 Jan 2020
 *      Author: Casper
 */

#include "Bootloader.h"
#include "DSerial.h"

extern DSerial serial;

Bootloader::Bootloader(MB85RS &fram){
    this->fram = &fram;
}

void Bootloader::JumpSlot(){
    uint8_t target_slot = 0;
    //this->fram->write(FRAM_TARGET_SLOT, &target_slot, 1);
    this->fram->read(FRAM_TARGET_SLOT, &target_slot, 1);


    asm("   MOV R0, PC\n"
        "   MOV R1, #0x20000000\n"
        "   STR R0, [R1]");

    uint32_t* pcPoint = (uint32_t*)0x20000000;

    uint8_t current_slot = (uint8_t)(*pcPoint >> 16);
    if(current_slot != 0) {
        current_slot--;
    }
    *pcPoint = current_slot;

    serial.print("BOOTLOADER: Current slot: ");
    serial.println(current_slot, DEC);
    serial.print("BOOTLOADER: Target slot: ");
    serial.println((target_slot & 0x7F), DEC);
    serial.print("BOOTLOADER: Permanently: ");
    serial.println(((target_slot & BOOT_PERMANENTLY) > 0) ? "YES" : "NO");

    if(current_slot != (target_slot & 0x7F)) {
        serial.print("BOOTLOADER: Jumping to slot:");
        serial.println((target_slot & 0x7F), DEC);

        MAP_Interrupt_disableMaster();
        MAP_WDT_A_holdTimer();

        uint32_t* resetPtr = 0;
        switch((target_slot & 0x7F)) {
            case 0:
                resetPtr = (uint32_t*)(0x00000 + 4);
                break;
            case 1:
                resetPtr = (uint32_t*)(0x20000 + 4);
                break;
            case 2:
                resetPtr = (uint32_t*)(0x30000 + 4);
                break;
            default:
                serial.println("BOOTLOADER: Error, target slot out of reach!");
                target_slot = BOOT_PERMANENTLY;
                this->fram->write(FRAM_TARGET_SLOT, &target_slot, 1);
                MAP_SysCtl_rebootDevice();
                break;
        }

        void (*slotPtr)(void) = (void (*)())(*resetPtr);
        slotPtr();
        while(1);
    } else {
        if((target_slot & BOOT_PERMANENTLY) == 0) {
            serial.println("BOOTLOADER: Only boot once in this slot. Resetting target to slot 0.");
            target_slot = BOOT_PERMANENTLY;
            this->fram->write(FRAM_TARGET_SLOT, &target_slot, 1);
        } else {
            serial.println("BOOTLOADER: Permanently boot from this slot.");
        }
    }
}


