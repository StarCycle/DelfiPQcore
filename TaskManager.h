/*
 * TaskManager.h
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_

#include "Task.h"

class TaskManager
{
protected:

public:
    static void start( Task **tasksArray, int count );
};

#endif /* TASKMANAGER_H_ */
