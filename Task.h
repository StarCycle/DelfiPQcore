/*
 * Task.h
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#ifndef TASK_H_
#define TASK_H_

#include <driverlib.h>

class Task
{
private:
    // allow TaskManager to access private functions (executeTask)
    friend class TaskManager;
    void executeTask( void );

 protected:
    volatile bool execute = false;
    void (*userFunction)( void );
    void (*initializerFunction)( void );

    virtual void run();

 public:
    Task( void );
    Task( void (*function)( void ) );
    Task( void (*function)( void ), void (*init)( void ) );
    virtual ~Task( void ) {};
    void notify( void );

    virtual bool notified( void );
    virtual void setUp( void );
};

#endif /* TASK_H_ */
