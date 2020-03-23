/*
 * PeriodicTask.h
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#ifndef PERIODICTASK_H_
#define PERIODICTASK_H_

#include <driverlib.h>
#include "Task.h"

class PeriodicTask : public Task
{
private:


public:
    PeriodicTask( const unsigned int count, void (*function)( void ), void (&init)( void ) );
    PeriodicTask( const unsigned int count, void (*function)( void ) );

    int taskCount = 0;
};

#endif /* PERIODICTASK_H_ */
