/*
 * ResetService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#include "ResetService.h"

extern DSerial serial;


/**
 *
 *   Reset device, triggered by WatchDog interrupt (time-out)
 *
 *   Parameters:

 *   Returns:
 *
 */
void resetHandler()
{
    serial.println("ResetService: internal watch-dog reset...");
    // TODO: replace this with a power cycle to protect also the RS485 driver
    // for now, at least reset, till the power cycle gets implemented in HW
    MAP_SysCtl_rebootDevice();
}

/**
 *
 *   ResetService Constructor
 *
 *   Parameters:
 *   WDport                     External watch-dog reset port
 *   WDpin                      External watch-dog reset pin
 *
 *   Returns:
 *
 */
ResetService::ResetService(const unsigned long WDport, const unsigned long WDpin) :
        WDIPort(WDport), WDIPin(WDpin) {}

/**
 *
 *   ResetService Initialize watch-dog service and interrupts.
 *
 *   Parameters:

 *   Returns:
 *
 */
void ResetService::init()
{
    // initialize the internal watch-dog
    MAP_WDT_A_clearTimer();                                  // Clear the watch-dog to prevent spurious triggers
    MAP_WDT_A_initIntervalTimer( WDT_A_CLOCKSOURCE_SMCLK,    // set the watch-dog to trigger every 178s
                                 WDT_A_CLOCKITERATIONS_2G ); // (about 3 minutes)

    // select the interrupt handler
    MAP_WDT_A_registerInterrupt(&resetHandler);

    // initialize external watch-dog pins
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setAsOutputPin( WDIPort, WDIPin );

    // start the timer
    MAP_WDT_A_startTimer();
}

/**
 *
 *   Reset internal watch-dog interrupt and timer and set external watch-dog port/pins as output.
 *
 *   Parameters:

 *   Returns:
 *
 */
void ResetService::refreshConfiguration()
{
    // select the interrupt handler
    MAP_WDT_A_registerInterrupt(&resetHandler);

    // ensure the timer is running: this only forces the
    // timer to run (in case it got disabled for any reason)
    // but it does not reset it, making sure the watch-dog
    // cannot be disabled by mistake
    MAP_WDT_A_startTimer();

    // initialize external watch-dog pins
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setAsOutputPin( WDIPort, WDIPin );
}

/**
 *
 *   Kick internal watchdog by resetting timer, should be called every (3min) or reset.
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::kickInternalWatchDog()
{
    // reset the internal watch-dog timer
    MAP_WDT_A_clearTimer();
}

/**
 *
 *   Kick external watch-dog by resetting timer.
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::kickExternalWatchDog()
{
    // toggle the WDI pin of the external watch-dog
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
}

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
bool ResetService::process(PQ9Frame &command, DataBus &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == RESET_SERVICE)
    {
        // prepare response frame
        workingBuffer.setDestination(command.getSource());
        workingBuffer.setSource(interface.getAddress());
        workingBuffer.setPayloadSize(3);
        workingBuffer.getPayload()[0] = RESET_SERVICE;

        if ((command.getPayloadSize() == 3) && (command.getPayload()[1] == RESET_REQUEST))
        {
            serial.println("ResetService: Reset");
            workingBuffer.getPayload()[2] = command.getPayload()[2];
            switch(command.getPayload()[2])
            {
                case RESET_SOFT:
                    workingBuffer.getPayload()[1] = RESET_RESPONSE;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    interface.transmit(workingBuffer);

                    // now reset the MCU
                    forceSoftReset();
                    break;

                case RESET_HARD:
                    workingBuffer.getPayload()[1] = RESET_RESPONSE;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    interface.transmit(workingBuffer);

                    // now force the external watch-dog to reset the MCU
                    forceHardReset();
                    break;

                    // not implemented yet, give error to notify it
                /*case RESET_POWERCYCLE:
                    serial.println("POWERCYCLE");
                    workingBuffer.getPayload()[1] = RESET_RESPONSE;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    interface.transmit(workingBuffer);
                    break;*/

                default:
                    workingBuffer.getPayload()[1] = RESET_ERROR;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    interface.transmit(workingBuffer);
                    break;
            }
        }
        else
        {
            // unknown request
            workingBuffer.getPayload()[1] = RESET_ERROR;
            // send response: doing it here to make sure
            // a response is sent before reset but not 2
            interface.transmit(workingBuffer);
        }

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

/**
 *
 *   Force the external watch-dog to reset the MCU
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::forceHardReset()
{
    // toggle the WDI pin 3 times, just to be sure
    // the external watch-dog resets...
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
}

/**
 *
 *   Force the internal watch-dog to reset the MCU
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::forceSoftReset()
{
    MAP_SysCtl_rebootDevice();
}
