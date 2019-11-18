/*
 * DelfiPQcore.h
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#ifndef DELFIPQCORE_H_
#define DELFIPQCORE_H_

#include <driverlib.h>
#include "msp.h"
#include "Task.h"

#define FCLOCK 48000000

class DelfiPQcore
{
public:
    static void initMCU();
};

#endif /* DELFIPQCORE_H_ */
