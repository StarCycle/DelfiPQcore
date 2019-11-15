/*
 * Task.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include "Task.h"

/**
 *
 *   Task Constructor:
 *
 *   Parameters:
 *   void (*function)           Function called by Task
 *   void (*init)               Function to initialise Task
 *
 *   Returns:
 *
 */
Task::Task( void (*function)( void ), void (*init)( void ) ) :
            userFunction(function), initializer(init) {}

/**
 *
 *   Task Constructor:
 *
 *   Parameters:
 *   void (*function)           Function called by Task (no initializer)
 *
 *   Returns:
 *
 */
Task::Task( void (*function)( void )) :
            userFunction(function), initializer([]{ }) {}

/**
 *
 *   Set flag that Task is ready for execution:
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void Task::notify( void )
{
    execute = true;
}

/**
 *
 *   Get flag that Task is ready for execution:
 *
 *   Parameters:
 *
 *   Returns:
 *    bool execute      :       True: Task is ready for Execution
 *                              False: Task is not ready for Execution
 */
bool Task::notified()
{
    return execute;
}

/**
 *
 *   Execute the Task (if execute flag is up) and lower execute flag
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
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

/**
 *
 *   Initialise Task using initialiser function (passed in contructor)
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void Task::setUp()
{
    initializer();
}
