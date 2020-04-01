/*
 * PingService.cpp
 *
 *  Created on: 25 Jul 2019
 *      Author: stefanosperett
 */
#include "PingService.h"


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
bool PingService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getPayload()[0] == PING_SERVICE) //Check if this frame is directed to this service
    {
        // prepare response frame
        //workingBuffer.setDestination(command.getSource());
        //workingBuffer.setSource(interface.getAddress());
        workingBuffer.setSize(command.getSize());
        workingBuffer.getPayload()[0] = PING_SERVICE;

        if (command.getPayload()[1] == SERVICE_RESPONSE_REQUEST)
        {
            Console::log("PingService: Ping Request");
            // respond to ping
            workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;
        }
        else
        {
            // unknown request
            workingBuffer.getPayload()[1] = SERVICE_RESPONSE_ERROR;
        }

        // send response
        //interface.transmit(workingBuffer);
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
