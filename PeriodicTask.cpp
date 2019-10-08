/*
 * PeriodicTask.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include <PeriodicTask.h>

PeriodicTask *instance;

static void placeholderCallback( void )
{

}

void timerHandler(void)
{
    MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
    instance->notify();
}

PeriodicTask::PeriodicTask(const unsigned int count, void (&function)( void ), void (&init)( void )) :
        Task(function, init)
{
    instance = this;
    // Configuring Timer32 to FCLOCK (1s) of MCLK in periodic mode
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
            TIMER32_PERIODIC_MODE);
    MAP_Timer32_registerInterrupt(TIMER32_0_INTERRUPT, &timerHandler);
    MAP_Timer32_setCount(TIMER32_0_BASE, count);
    MAP_Timer32_startTimer(TIMER32_0_BASE, false);
}

PeriodicTask::PeriodicTask(const unsigned int count, void (&function)( void )) :
        PeriodicTask(count, function, placeholderCallback) {}
