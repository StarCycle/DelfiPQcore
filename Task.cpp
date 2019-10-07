/*
 * Task.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include "Task.h"

Task::Task( void (&function)( void ) ) :
            userFunction(function) {}

void Task::notify( void )
{
    execute = true;
}

bool Task::notified()
{
    return execute;
}

void Task::executeTask()
{
    if (execute)
    {
        execute = false;
        if (userFunction)
        {
            userFunction();
        }
    }
}
