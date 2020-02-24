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

    //Program Counter is either at: (0x000000XX, 0x000200XX, 0x000300XX)
    //meaning you can detect the current slot by looking at the 17th bit
    uint8_t current_slot = (uint8_t)(*pcPoint >> 16);
    switch(current_slot){
        case 0x00:
            current_slot = 0;
            break;
        case 0x02:
            current_slot = 1;
            break;
        case 0x03:
            current_slot = 2;
            break;
    }

    serial.println("================= BOOTLOADER ================");
    serial.print("= Current slot: ");
    serial.println(current_slot, DEC);

    if(current_slot == 0 && (target_slot & 0x7F) != 0){ //if we are in slot0 (default slot) we might have to jump
        serial.print("= Target slot: ");
        serial.println((target_slot & 0x7F), DEC);
        serial.print("= Permanent Jump: ");
        serial.println(((target_slot & BOOT_PERMANENTLY) > 0) ? "YES" : "NO"); //permanent jump flag is set (not a one time jump)

        if((target_slot & BOOT_PERMANENTLY) == 0) {
            serial.println("+ Preparing One-time jump");
            this->fram->write(FRAM_TARGET_SLOT, &current_slot, 1); //reset target to slot0
        } else {
            serial.println("+ Preparing Permanent jump");
        }

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
                serial.println("+ BOOTLOADER - Error: target slot not valid!");
                target_slot = BOOT_PERMANENTLY; //set target to 0 and reboot
                this->fram->write(FRAM_TARGET_SLOT, &target_slot, 1);
                MAP_SysCtl_rebootDevice();
                break;
        }


        serial.println("=============================================");
        void (*slotPtr)(void) = (void (*)())(*resetPtr);
        slotPtr();  //This is the jump!
        while(1);

    }else{
        serial.println("=============================================");
    }
}


