/*
 * PeriodicTaskNotifier.cpp
 *
 *  Created on: 21 Jan 2020
 *      Author: Casper
 */

#include "PeriodicTaskNotifier.h"

PeriodicTaskNotifier *notifierStub;

void NotifyTasks_stub(){
    //Temporary Fix for WDT Kicking too fast
    MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
    notifierStub->NotifyTasks();
};

PeriodicTaskNotifier::PeriodicTaskNotifier(int count, Task** taskListIn, int nrOfTasks ) :
    taskList(taskListIn), numberOfTasks(nrOfTasks){
    notifierStub = this;
//    MAP_SysTick_enableModule();
//    MAP_SysTick_setPeriod(count);  << TODO: count is only 24-bits
//    MAP_SysTick_registerInterrupt(&NotifyTasks_stub);
//    MAP_SysTick_enableInterrupt();

    //Temporary Fix for WDT Kicking too fast
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                TIMER32_PERIODIC_MODE);
    MAP_Timer32_registerInterrupt(TIMER32_0_INTERRUPT, &NotifyTasks_stub);
    MAP_Timer32_setCount(TIMER32_0_BASE, count);
    MAP_Timer32_startTimer(TIMER32_0_BASE, false);
};


void PeriodicTaskNotifier::NotifyTasks(){
    for(int k = 0; k < numberOfTasks; k++){
        taskList[k]->notify();
    }
}

