/*
 * PeriodicTaskNotifier.h
 *
 *  Created on: 21 Jan 2020
 *      Author: Casper
 */

#ifndef PERIODICTASKNOTIFIER_H_
#define PERIODICTASKNOTIFIER_H_

#include "Task.h"

class PeriodicTaskNotifier
{
private:
    Task** taskList;
    int numberOfTasks;

public:
    PeriodicTaskNotifier(int count, Task** taskListIn, int nrOfTasks );
    void NotifyTasks();
};




#endif /* PERIODICTASKNOTIFIER_H_ */
