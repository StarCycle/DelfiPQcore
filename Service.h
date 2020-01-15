/*
 * Service.h
 *
 *  Created on: 25 Jul 2019
 *      Author: stefanosperett
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include "DataBus.h"
#include "DataFrame.h"

class Service
{
 protected:

 public:

    virtual ~Service( ) {};
    virtual bool process(DataFrame &command, DataBus &interface, DataFrame &workingBbuffer) = 0;
};

#endif /* SERVICE_H_ */
