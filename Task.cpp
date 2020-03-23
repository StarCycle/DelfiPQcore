/*
 * Task.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include "Task.h"

/**
 *
 *   Task Constructor: take a user and initializer functions
 *
 *   Parameters:
 *   void (*userFunction)           User function called by the task
 *   void (*initializerFunction)    Initializer function to initialize the task
 *
 *   Returns:
 *
 */
Task::Task( void (*function)( void ), void (*init)( void ) ) :
            userFunction( function ), initializerFunction( init ) {}

/**
 *
 *   Task Constructor: take a user function
 *
 *   Parameters:
 *   void (*userFunction)          User function called by the task
 *
 *   Returns:
 *
 */
Task::Task( void (*function)( void )) :
            userFunction( function ), initializerFunction( 0 ) {}

/**
 *
 *   Task Constructor: the user and the initializer functions
 *   should be inherited from Task.h
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
Task::Task( void ) : userFunction( 0 ), initializerFunction( 0 ) {}

/**
 *
 *   Weak up the task
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void Task::notify( void )
{
    execute = true;

    // make sure the task manager is awaken
    MAP_Interrupt_disableSleepOnIsrExit();
}

/**
 *
 *   Return if the Task should weak up
 *
 *   Parameters:
 *
 *   Returns:
 *   bool execute               True: Task is ready for Execution
 *                              False: Task is not ready for Execution
 */
bool Task::notified( void )
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
void Task::executeTask( void )
{
    if (notified())
    {
        run();
        execute = false;
    }
}

/**
 *
 *   Task function (passed in constructor or overridden)
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void Task::run( void )
{
    if (userFunction)
    {
        userFunction();
    }
}

/**
 *
 *   Initialize Task using initializer function (passed in constructor or overridden)
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void Task::setUp( void )
{
    if (initializerFunction)
    {
        initializerFunction();
    }
}
