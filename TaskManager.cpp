/*
 * TaskManager.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include "TaskManager.h"

/**
 *
 *   Start Task Execution
 *
 *   Parameters:
 *
 *   Returns:
 *
 *
 */
void TaskManager::start( Task **tasks, int count )
{
    // run the initialization code first
    for (int i = 0; i < count; i++)
    {
        tasks[i]->setUp();
    }

    // run all tasks in a sequence
    while(true)
    {
        // loop through all tasks
        for (int i = 0; i < count; i++)
        {
            tasks[i]->executeTask();
        }

        // check if any of the tasks has been notified in the meantime
//        bool notified = false;
//        for (int i = 0; i < count; i++)
//        {
//            notified |= tasks[i]->notified();
//        }

        // if no task has been notified in the meantime, go to sleep
        // otherwise execute the tasks again
        //if (!notified)
        {
            // go into low-power mode
            //MAP_Interrupt_enableSleepOnIsrExit();
            //MAP_PCM_gotoLPM0InterruptSafe();
        }
    }
}
