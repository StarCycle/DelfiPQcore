/*
 * TaskManager.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include <TaskManager.h>

/**
 *
 *   Contruct Task Manager
 *
 *   Parameters:
 *   Task **tasksArray  Array of Task objects
 *   int count          Amount of Tasks
 *
 *   Returns:
 *
 *
 */
TaskManager::TaskManager(Task **tasksArray, int count) :
        tasks(tasksArray), tasksCount(count) {}

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
void TaskManager::start()
{
    while(true)
    {
        for (int i = 0; i < tasksCount; i++)
        {
            tasks[i]->executeTask();
        }

        //MAP_PCM_gotoLPM0();
    }
}
