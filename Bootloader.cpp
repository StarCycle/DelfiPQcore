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
    this->current_slot = this->getCurrentSlot();
}

unsigned char Bootloader::getCurrentSlot(){
    uint8_t slotNumber = 0;

    asm("   MOV R0, PC\n"
            "   MOV R1, #0x20000000\n"
            "   STR R0, [R1]");

    uint32_t* pcPoint = (uint32_t*)0x20000000;
//    serial.print("= PC: ");
//    serial.println(*pcPoint, DEC);
    //Program Counter is either at: (0x000000XX, 0x000200XX, 0x000300XX)
    //meaning you can detect the current slot by looking at the 17th bit
    slotNumber = (uint8_t)(*pcPoint >> 16);
    switch(slotNumber){
        case 0x00:
            slotNumber = 0;
            break;
        case 0x02:
            slotNumber = 1;
            break;
        case 0x03:
            slotNumber = 2;
            break;
        default: //should never happen
            slotNumber = 0;
            break;
    }
    return slotNumber;
}
void Bootloader::JumpSlot(){
    uint8_t target_slot = 0;
    //this->fram->write(FRAM_TARGET_SLOT, &target_slot, 1);
    serial.println("================= BOOTLOADER ================");

    current_slot = this->getCurrentSlot();

    serial.print("= Current slot: ");
    serial.println(current_slot, DEC);

    if(fram->ping()){
        this->fram->read(BOOTLOADER_TARGET_REG, &target_slot, 1);

        if((current_slot & 0x7F) == 0 && (target_slot & 0x7F) != 0){ //is in main slot and preparing to 'jump'
            //check nr of Reboots to reset targetslot to 0
           uint8_t nrOfReboots = 0;
           fram->read(FRAM_RESET_COUNTER + (target_slot & 0x7F), &nrOfReboots, 1); //get nr 'surprise' reboots of targets
           serial.print("= Target: ");
           serial.println((target_slot & 0x7F),DEC);
           serial.print("= Number of Reboots Target: ");
           serial.println(nrOfReboots, DEC);
           if(nrOfReboots > 10 && ((target_slot & 0x7F) != 0)){ //if the surprise reboots >10, reset boot target and reset reboot counter
               serial.println("# Max amount of unintentional reboots!");
               serial.println("# Resetting TargetSlot");
               nrOfReboots = 0;
               fram->write(FRAM_RESET_COUNTER + (target_slot & 0x7F), &nrOfReboots, 1);
               fram->write(BOOTLOADER_TARGET_REG, &current_slot, 1); //reset target to slot0
               fram->read(BOOTLOADER_TARGET_REG, &target_slot, 1);
           }

            //check Succesful boot flag for problems
            uint8_t succesfulBootFlag = 0;
            fram->read(FRAM_BOOT_SUCCES_FLAG, &succesfulBootFlag, 1);
            if(succesfulBootFlag == 0){ //Boot is not succesful, fallback on default slot.
                serial.println("# Last Boot unsuccesful, resetting TargetSlot");
                this->fram->write(BOOTLOADER_TARGET_REG, &current_slot, 1); //reset target to slot0
                this->fram->read(BOOTLOADER_TARGET_REG, &target_slot, 1);
                succesfulBootFlag = 1; //reset bootflag.
                fram->write(FRAM_BOOT_SUCCES_FLAG, &succesfulBootFlag, 1);
            }

            //No problems encountered prep for jump if target is still set
            if((target_slot & 0x7F) != 0){
                serial.print("= Target slot: ");
                serial.println((target_slot & 0x7F), DEC);
                serial.print("= Permanent Jump: ");
                serial.println(((target_slot & BOOT_PERMANENT_FLAG) > 0) ? "YES" : "NO"); //permanent jump flag is set (not a one time jump)

                if((target_slot & BOOT_PERMANENT_FLAG) == 0) {
                    serial.println("+ Preparing One-time jump");
                    this->fram->write(BOOTLOADER_TARGET_REG, &current_slot, 1); //reset target to slot0
                } else {
                    serial.println("+ Preparing Permanent jump");
                    //this->fram->write(FRAM_TARGET_SLOT, &current_slot, 1); //reset target to slot0
                }

                //lowerBootSuccesFlag before jump
                uint8_t succesfulBootFlag = 0;
                this->fram->write(FRAM_BOOT_SUCCES_FLAG, &succesfulBootFlag, 1);

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
                        target_slot = BOOT_PERMANENT_FLAG; //set target to 0 and reboot
                        this->fram->write(BOOTLOADER_TARGET_REG, &target_slot, 1);
                        MAP_SysCtl_rebootDevice();
                        break;
                }
                serial.print("Jumping to: ");
                serial.println(*resetPtr, HEX);
                serial.println("=============================================");

                void (*slotPtr)(void) = (void (*)())(*resetPtr);

                slotPtr();  //This is the jump!

                while(1){
                    serial.println("Why are we here?"); //should never end up here
                }
            }else{
                //not jumping anymore
                serial.println("=============================================");
            }

        }else if((current_slot & 0x7F) != 0){
            //In target slot succesfully, hence it is a succesful boot
            uint8_t succesfulBootFlag = 1; //reset bootflag.
            fram->write(FRAM_BOOT_SUCCES_FLAG, &succesfulBootFlag, 1);
            serial.println("=============================================");
        }else{ //In the default slot, but no target is set
            serial.println("=============================================");
        }
    }else{ //fram did not ping
        serial.println("# FRAM Unavailable!");
        serial.println("=============================================");
    }
}


