/*
 * Copyright (c) 2016 by Stefano Speretta <s.speretta@tudelft.nl>
 *
 * PQ9CommandHandler: a library to handle PQ9 commands.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 3, both as published by the Free Software Foundation.
 *
 */

#include <CommandHandler.h>

extern DSerial serial;

/**
 *
 *   CommandHandler Constructor
 *
 *   Parameters:
 *   PQ9Sender &interface       Physical Bus object
 *   Service **servArray        Array of services (commands) to handle
 *   int count                  Amount of services in Array
 *
 *   Returns:
 *
 */
PQ9CommandHandler::PQ9CommandHandler(PQ9Sender &interface, Service **servArray, int count) :
          Task(), bus(interface), services(servArray), servicesCount(count)
{
    onValidCmd = 0;
}

/**
 *
 *   Receive Commands from Bus (Should be hooked to bus .SetReceiveHandler())
 *      and Set Task Execution Flag
 *
 *   Parameters:
 *   PQ9Frame &newFrame         Received Frame from bus
 *
 *   Returns:
 *
 */
void PQ9CommandHandler::received( PQ9Frame &newFrame )
{
    newFrame.copy(rxBuffer);
    notify();
}

/**
 *
 *   Called when a Valid Command was received over Bus and executed
 *
 *   Parameters:
 *   void (*function)         Function invoked upon receival and execution of valid Command
 *
 *   Returns:
 *
 */
void PQ9CommandHandler::onValidCommand(void (*function)( void ))
{
    onValidCmd = function;
}

/**
 *
 *   Handle received commands (called by CommandHandler)
 *
 *   Parameters:
 *
 *   Returns:
 *   True: Command was Handled
 *   False: Command was not Handled
 *
 */
void PQ9CommandHandler::run()
{
    if (rxBuffer.getPayloadSize() > 1)
    {
        bool found = false;

        for (int i = 0; i < servicesCount; i++)
        {
            if (services[i]->process(rxBuffer, bus, txBuffer)) // Does any of the Services Handle this command?
            {
                // stop the loop if a service is found
                found = true;
                break;
            }
        }

        if (!found)
        {
            serial.print("Unknown Service (");
            serial.print(rxBuffer.getPayload()[0], DEC);
            serial.println(")");
            txBuffer.setDestination(rxBuffer.getSource());
            txBuffer.setSource(bus.getAddress());
            txBuffer.setPayloadSize(2);
            txBuffer.getPayload()[0] = 0;
            txBuffer.getPayload()[1] = 0;
            bus.transmit(txBuffer);
            return;
        }
        else
        {
            if (onValidCmd)
            {
                onValidCmd();
            }
            return;
        }
    }
    else
    {
        // invalid payload size
        // what should we do here?
        serial.println("Invalid Command, size must be > 1");
        txBuffer.setDestination(rxBuffer.getSource());
        txBuffer.setSource(bus.getAddress());
        txBuffer.setPayloadSize(2);
        txBuffer.getPayload()[0] = 0;
        txBuffer.getPayload()[1] = 0;
        bus.transmit(txBuffer);
        return;
    }
}
