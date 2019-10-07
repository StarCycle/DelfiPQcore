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
    Task** tasks;
    const int tasksCount;

public:
    TaskManager( Task **tasksArray, int count );
    virtual ~TaskManager();
    void start( void );
};

#endif /* TASKMANAGER_H_ */
