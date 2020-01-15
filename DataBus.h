/*
 * DataBus.h
 *
 *  Created on: 14 Jan 2020
 *      Author: CasperBroekhuizen
 */

#ifndef _DATABUS_H_
#define _DATABUS_H_

#include "DataFrame.h"

class DataBus
{
public:
    virtual ~DataBus(){}
    virtual void transmit( DataFrame &frame ) = 0;
    virtual unsigned char getAddress( void ) = 0;
    virtual void setReceiveHandler( void (*islHandle)( DataFrame & ) ) = 0;
};

#endif  /* _DATABUS_H_ */
