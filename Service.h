/*
 * Service.h
 *
 *  Created on: 25 Jul 2019
 *      Author: stefanosperett
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include "PQ9Sender.h"
#include "PQ9Frame.h"

class Service
{
 protected:

 public:

    virtual ~Service( ) {};
    virtual bool process(PQ9Frame &command, PQ9Sender &interface, PQ9Frame &workingBbuffer) = 0;
};

#endif /* SERVICE_H_ */
