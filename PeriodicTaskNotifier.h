/*
 * PeriodicTaskNotifier.h
 *
 *  Created on: 21 Jan 2020
 *      Author: Casper
 */

#ifndef PERIODICTASKNOTIFIER_H_
#define PERIODICTASKNOTIFIER_H_

#define MAX_PERIODIC_TASKS 50


#include "PeriodicTask.h"
#include "DelfiPQCore.h"
#include "Console.h"

class PeriodicTaskNotifier
{
private:
    PeriodicTask** taskList;
    int numberOfTasks;
    int taskCounter[MAX_PERIODIC_TASKS] = {0};

    int count;

public:
    PeriodicTaskNotifier(PeriodicTask** taskListIn, int nrOfTasks );
    void NotifyTasks();
    void init();
};




#endif /* PERIODICTASKNOTIFIER_H_ */
