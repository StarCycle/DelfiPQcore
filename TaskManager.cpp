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
        // low-power mode currently disabled
        // TODO: RS485 frames lost sometimes when low-power mode enables
        //MAP_PCM_gotoLPM0();
    }
}
