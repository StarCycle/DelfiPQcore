/*
 * Task.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include "Task.h"

Task::Task( void (*function)( void ), void (*init)( void ) ) :
            userFunction(function), initializer(init) {}

Task::Task( void (*function)( void )) :
            userFunction(function), initializer([]{ }) {}

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
        if (userFunction)
        {
            userFunction();
        }
        execute = false;
    }
}

void Task::setUp()
{
    initializer();
}
