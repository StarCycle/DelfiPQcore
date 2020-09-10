/*
 * FRAMService.cpp
 *
 *  Created on: 23 Jul 2020
 *      Author: Casper
 */

#include "FRAMService.h"

FRAMService::FRAMService(MB85RS& fram_in){
    fram = &fram_in;
}

bool FRAMService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getService() == FRAM_SERVICE)
    {
        Console::log("FRAMService");
        workingBuffer.setService(FRAM_SERVICE);
        workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);

        uint8_t result = 0;
        uint32_t address = 0;

        switch(command.getDataPayload()[0]){
        case 0:
            uint8_t ping = fram->ping();
            Console::log("Ping: %d", ping);
            if(ping){
                unsigned long id = fram->getID();
                Console::log("ID: %x", id);
            }
            workingBuffer.setPayloadSize(2);
            workingBuffer.getDataPayload()[0] = 0;
            workingBuffer.getDataPayload()[1] = ping;

            break;
        case 1:
            if(command.getPayloadSize() == 5){
                Console::log("Read");
                result = 0;
                address = 0;
                ((uint8_t*) &address)[3] = command.getDataPayload()[1];
                ((uint8_t*) &address)[2] = command.getDataPayload()[2];
                ((uint8_t*) &address)[1] = command.getDataPayload()[3];
                ((uint8_t*) &address)[0] = command.getDataPayload()[4];
                fram->read(address, &result, 1);
                Console::log("Address: %d | Value: %d", address, result);
                workingBuffer.setPayloadSize(2);
                workingBuffer.getDataPayload()[0] = 0;
                workingBuffer.getDataPayload()[1] = result;
            }else{
                //invalid command
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = FRAM_SERVICE_INVALID_CMD;
            }
            break;
        case 2:
            if(command.getPayloadSize() >= 6){
                Console::log("Write");
                address = 0;
                ((uint8_t*) &address)[3] = command.getDataPayload()[1];
                ((uint8_t*) &address)[2] = command.getDataPayload()[2];
                ((uint8_t*) &address)[1] = command.getDataPayload()[3];
                ((uint8_t*) &address)[0] = command.getDataPayload()[4];
                Console::log("Address: %d | nr of values: %d", address, command.getPayloadSize() - 5);
                fram->write(address, &command.getDataPayload()[5], command.getPayloadSize() - 5);
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = 0;
            }else{
               //invalid command
               workingBuffer.setPayloadSize(1);
               workingBuffer.getDataPayload()[0] = FRAM_SERVICE_INVALID_CMD;
           }
            break;
        case 3:
            Console::log("Erase all");
            fram->erase();
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = 0;
            break;
        case 4:
            if(command.getPayloadSize() >= 6){
                Console::log("Write and Reset");
                address = 0;
                ((uint8_t*) &address)[3] = command.getDataPayload()[1];
                ((uint8_t*) &address)[2] = command.getDataPayload()[2];
                ((uint8_t*) &address)[1] = command.getDataPayload()[3];
                ((uint8_t*) &address)[0] = command.getDataPayload()[4];
                Console::log("Address: %d | nr of values: %d", address, command.getPayloadSize() - 5);
                fram->write(address, &command.getDataPayload()[5], command.getPayloadSize() - 5);
    #if defined (__MSP432P401R__)
                this->setPostFunc([](){MAP_SysCtl_rebootDevice();});
    #elif defined (__MSP432P4111__)
                this->setPostFunc([](){MAP_SysCtl_A_rebootDevice();});
    #endif
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = 0;
            }else{
                //invalid command
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = FRAM_SERVICE_INVALID_CMD;
            }
            break;
        case 5:
            Console::log("Erase all and Reboot");
            fram->erase();
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = 0;
#if defined (__MSP432P401R__)
            this->setPostFunc([](){MAP_SysCtl_rebootDevice();});
#elif defined (__MSP432P4111__)
            this->setPostFunc([](){MAP_SysCtl_A_rebootDevice();});
#endif
            break;
        default:
            Console::log("Unknown Command");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = FRAM_SERVICE_INVALID_CMD;
            break;
        }
        return true;
    }
    return false;
}


