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
    uint8_t jumpSlot = 0;
    uint8_t targetSlot = 0;
    this->fram->read(0x7FF0, &targetSlot, 1);
    this->fram->read(0x7FF1, &jumpSlot, 1);


    asm("   MOV R0, PC\n"
        "   MOV R1, #0x20000000\n"
        "   STR R0, [R1]");

    uint32_t* pcPoint = (uint32_t*) 0x20000000;
    serial.print("BOOTLOADER: PC: ");
    serial.println(*pcPoint, HEX);
    serial.print("BOOTLOADER: Slot: ");
    switch (*pcPoint >> 16) {
        case 0:
            serial.println('0');
            break;
        case 2:
            serial.println('1');
            break;
        case 3:
            serial.println('2');
            break;
        default:
            serial.println('unknown');
            break;
    }

    if(jumpSlot == 1){
        jumpSlot = 0;
        this->fram->write(0x7FF1, &jumpSlot, 1);
        serial.println("BOOTLOADER: Getting ready to jump!");
        serial.print("BOOTLOADER: Jumping to slot:");
        serial.println(targetSlot, DEC);
        MAP_Interrupt_disableMaster();
        MAP_WDT_A_holdTimer();
        uint32_t* resetPtr = 0; //get only lowerBits
        if(targetSlot == 0){
            resetPtr = (uint32_t*)(0x00000 + 4);
        }
        else if(targetSlot == 1){
            resetPtr = (uint32_t*)(0x20000 + 4);
        }
        else if(targetSlot == 2){
            resetPtr = (uint32_t*)(0x30000 + 4);
        }
        else{
            serial.println("BOOTLOADER: Error, not supposed to happen!");
            MAP_SysCtl_rebootDevice();
        }
        void (*slot1Ptr)(void) = (void (*)())(*resetPtr);
        slot1Ptr();
        while(1);
    }else{
        serial.println("BOOTLOADER: Not Jumping!");
    }
}


