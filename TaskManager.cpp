/*
 * TaskManager.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include <TaskManager.h>

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

    // run the tasks in a sequence
    while(true)
    {
        for (int i = 0; i < count; i++)
        {
            tasks[i]->executeTask();
        }
        // TODO: what if a task weaks up another task?
        // go into low-power mode
        MAP_PCM_gotoLPM0();
    }
}
