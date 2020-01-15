/*
 * PingService.cpp
 *
 *  Created on: 25 Jul 2019
 *      Author: stefanosperett
 */
#include "PingService.h"

extern DSerial serial;

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   PQ9Bus &interface          Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */
bool PingService::process(PQ9Frame &command, DataBus &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == PING_SERVICE) //Check if this frame is directed to this service
    {
        // prepare response frame
        workingBuffer.setDestination(command.getSource());
        workingBuffer.setSource(interface.getAddress());
        workingBuffer.setPayloadSize(2);
        workingBuffer.getPayload()[0] = PING_SERVICE;

        if (command.getPayload()[1] == PING_REQUEST)
        {
            serial.println("PingService: Ping Request");
            // respond to ping
            workingBuffer.getPayload()[1] = PING_RESPONSE;
        }
        else
        {
            // unknown request
            workingBuffer.getPayload()[1] = PING_ERROR;
        }

        // send response
        interface.transmit(workingBuffer);
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
