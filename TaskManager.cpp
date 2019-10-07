/*
 * TaskManager.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include <TaskManager.h>

TaskManager::TaskManager(Task **tasksArray, int count) :
        tasks(tasksArray), tasksCount(count) {}

void TaskManager::start()
{
    while(true)
    {
        for (int i = 0; i < tasksCount; i++)
        {
            tasks[0]->executeTask();
        }

        //MAP_PCM_gotoLPM0();
    }
}
