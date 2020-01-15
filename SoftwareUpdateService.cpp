/*
 * SoftwareUpdateService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#include "SoftwareUpdateService.h"

extern DSerial serial;

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   DataBus &interface       Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */
bool SoftwareUpdateService::process(DataFrame &command, DataBus &interface, DataFrame &workingBuffer)
{

    if (command.getPayload()[0] == SOFTWAREUPDATE_SERVICE)
    {
        serial.println("SoftwareUpdate Service");

        // command processed
        return true;
    }
    else
    {
        // this command is related to another service,
        // report the command was not processed
        return false;
    }
}
