/*
 * PeriodicTaskNotifier.cpp
 *
 *  Created on: 21 Jan 2020
 *      Author: Casper
 */

#include "PeriodicTaskNotifier.h"

PeriodicTaskNotifier *notifierStub;

void NotifyTasks_stub(){
    notifierStub->NotifyTasks();
};

PeriodicTaskNotifier::PeriodicTaskNotifier(int count, Task** taskListIn, int nrOfTasks ) :
    taskList(taskListIn), numberOfTasks(nrOfTasks){
    notifierStub = this;
    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(count);
    MAP_SysTick_registerInterrupt(&NotifyTasks_stub);
    MAP_SysTick_enableInterrupt();
};


void PeriodicTaskNotifier::NotifyTasks(){
    for(int k = 0; k < numberOfTasks; k++){
        taskList[k]->notify();
    }
}

