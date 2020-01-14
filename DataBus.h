/*
 * DataBus.h
 *
 *  Created on: 14 Jan 2020
 *      Author: CasperBroekhuizen
 */

#ifndef _DATABUS_H_
#define _DATABUS_H_

#include "PQ9Frame.h"

class DataBus
{
public:
    virtual ~DataBus(){}
    virtual void transmit( PQ9Frame &frame ) = 0;
    virtual unsigned char getAddress( void ) = 0;
    virtual void setReceiveHandler( void (*islHandle)( PQ9Frame & ) ) = 0;
};

#endif  /* _DATABUS_H_ */
