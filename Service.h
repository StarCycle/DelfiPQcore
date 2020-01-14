/*
 * Service.h
 *
 *  Created on: 25 Jul 2019
 *      Author: stefanosperett
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include "DataBus.h"
#include "PQ9Frame.h"

class Service
{
 protected:

 public:

    virtual ~Service( ) {};
    virtual bool process(PQ9Frame &command, DataBus &interface, PQ9Frame &workingBbuffer) = 0;
};

#endif /* SERVICE_H_ */
