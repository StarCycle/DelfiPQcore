/*
 * Task.h
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#ifndef TASK_H_
#define TASK_H_

class Task
{
 protected:
    volatile bool execute = false;
    void (*userFunction)( void );
    void (*initializer)( void );

 public:
    Task( void (*function)( void ) );
    Task( void (*function)( void ), void (*init)( void ) );
    virtual ~Task( ) {};
    virtual void notify( void );
    virtual bool notified();
    virtual void executeTask();
    virtual void setUp();
};

#endif /* TASK_H_ */