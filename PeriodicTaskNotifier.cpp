/*
 * PeriodicTaskNotifier.cpp
 *
 *  Created on: 21 Jan 2020
 *      Author: Casper
 */

#include "PeriodicTaskNotifier.h"



PeriodicTaskNotifier *notifierStub;

void NotifyTasks_stub(){
    //systick interrupt clears automatically.
    notifierStub->NotifyTasks();
};

PeriodicTaskNotifier::PeriodicTaskNotifier(PeriodicTask** taskListIn, int nrOfTasks ) :
    taskList(taskListIn), numberOfTasks(nrOfTasks){
    notifierStub = this;

    count = FCLOCK/(1000/TASKNOTIFIER_PERIOD_MS);
    static_assert( (FCLOCK/(1000/TASKNOTIFIER_PERIOD_MS)) >> 24 == 0, "PeriodicTaskNotifier Period is only 24 bits!" ); //(assert this is less than 24 bits)

    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(count);  // count is only 24-bits
    MAP_SysTick_registerInterrupt(NotifyTasks_stub);
    MAP_SysTick_enableInterrupt();

    //clear the counter list
//    for(int k = 0; k < numberOfTasks; k++){
//        this->taskCounter[k] = 0;
//    }

};


void PeriodicTaskNotifier::NotifyTasks(){
    for(int k = 0; k < numberOfTasks; k++){
        this->taskCounter[k] += 1;
        if(this->taskCounter[k] >= this->taskList[k]->taskCount){
            this->taskCounter[k] = 0;
            taskList[k]->notify();
        }
    }
}

