/*
 * ResetService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#include "ResetService.h"

ResetService* resetServiceStub;

void _forcePowerCycle()
{
    resetServiceStub->forcePowerCycle();
}

void _forceHardReset()
{
    resetServiceStub->forceHardReset();
}

void _forceSoftReset()
{
    resetServiceStub->forceSoftReset();
}

/**
 *
 *   Reset device, triggered by WatchDog interrupt (time-out)
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void resetHandler()
{
    Console::log("ResetService: internal watch-dog reset... ");
    // make sure all characters have been flushed to the console before rebooting
    Console::flush( );

    resetServiceStub->forcePowerCycle();
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
        WDIPort(WDport), WDIPin(WDpin), ERPort(0), ERPin(0) {
    resetServiceStub = this;
}

/**
 *
 *   ResetService Constructor
 *
 *   Parameters:
 *   WDport                     External watch-dog reset port
 *   WDpin                      External watch-dog reset pin
 *   ERport                     External reset port
 *   ERpin                      External reset pin
 *
 *   Returns:
 *
 */
ResetService::ResetService(const unsigned long WDport, const unsigned long WDpin,
                           const unsigned long Rport, const unsigned long Rpin) :
        WDIPort(WDport), WDIPin(WDpin), ERPort(Rport), ERPin(Rpin) {
    resetServiceStub = this;
}

/**
 *
 *   ResetService Initialize watch-dog service and interrupts.
 *
 *   Parameters:
 *
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

    // initialize external watch-dog pin
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setAsOutputPin( WDIPort, WDIPin );

    // initialize power cycle pin
    MAP_GPIO_setOutputHighOnPin( ERPort, ERPin );
    MAP_GPIO_setAsOutputPin( ERPort, ERPin );

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
 *   Kick internal watch-dog by resetting timer, should be called every (3min) or reset.
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
bool ResetService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getPayload()[0] == RESET_SERVICE)
    {
        // prepare response frame
        workingBuffer.setSize(3);
        workingBuffer.getPayload()[0] = RESET_SERVICE;

        if (command.getPayload()[1] == SERVICE_RESPONSE_REQUEST)
        {
            workingBuffer.getPayload()[2] = command.getPayload()[2];
            switch(command.getPayload()[2])
            {
                case RESET_SOFT:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;

                    // after a response has been sent, reset the MCU
                    this->setPostFunc(_forceSoftReset);
                    break;

                case RESET_HARD:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;

                    // after a response has been sent, force the external watch-dog to reset the MCU
                    this->setPostFunc(_forceHardReset);
                    break;

                case RESET_POWERCYCLE:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;

                    // after a response has been sent, force a power cycle
                    this->setPostFunc(_forcePowerCycle);
                    break;

                default:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_ERROR;
                    break;
            }
        }
        else
        {
            // unknown request
            workingBuffer.getPayload()[1] = RESET_ERROR;
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
 *   Force a full board power cycle
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::forcePowerCycle()
{
    Console::log("ResetService: Power Cycle");
    // make sure all characters have been flushed to the console before rebooting
    Console::flush( );

    // if the power-cycle command is supported by the hardware,
    // trigger a power-cycle
    if (ERPort != 0)
    {
        MAP_GPIO_setOutputLowOnPin( ERPort, ERPin );
    }
    else
    {
        // Otherwise trigger a hard reset
        // Add WDT time-out to reset-cause register
        MAP_ResetCtl_initiateHardResetWithSource(RESET_HARD_WDTTIME);
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
    Console::log("ResetService: Hard reset");
    // make sure all characters have been flushed to the console before rebooting
    Console::flush( );

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
    Console::log("ResetService: Soft reset");
    // make sure all characters have been flushed to the console before rebooting
    Console::flush( );

#if defined (__MSP432P401R__)
    MAP_SysCtl_rebootDevice();
#elif defined (__MSP432P4111__)
    MAP_SysCtl_A_rebootDevice();
#endif

}
