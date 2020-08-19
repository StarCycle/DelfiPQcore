/*
 * PeriodicTask.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include "PeriodicTask.h"

PeriodicTask *instanceTimerTask;


/**
 *
 *   Construct PeriodicTask (extends Task)
 *
 *   Parameters:
 *      const unsigned int count        Period of the task in ms
 *      void (*function)                The function to Execute
 *      void (*init)                    The Initializer of the Function
 *   Returns:
 *
 *
 */
PeriodicTask::PeriodicTask(const unsigned int count, void (*function)( void ), void (*init)( void )) :
        Task(function, init)
{
    this->taskCount = (count/TASKNOTIFIER_PERIOD_MS) + ((count % TASKNOTIFIER_PERIOD_MS) != 0);
}

/**
 *
 *   Construct PeriodicTask (extends Task)
 *
 *   Parameters:
 *      const unsigned int count        Period of the task in ms
 *      void (*function)                The function to Execute
 *   Returns:
 *
 *
 */
PeriodicTask::PeriodicTask(const unsigned int count, void (*function)( void )) :
        Task(function)
{
    this->taskCount = count/TASKNOTIFIER_PERIOD_MS;
}
