/*
 * PQ9CommandHandler.h
 *
 *  Created on: 24 Jul 2019
 *      Author: stefanosperett
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "Service.h"
#include "Task.h"
#include "PingService.h"
#include "DSerial.h"

class PQ9CommandHandler: public Task
{
 protected:
     DataBus &bus;
     Service** services;
     int servicesCount;
     PQ9Frame rxBuffer, txBuffer;
     void (*onValidCmd)( void );
     virtual void run();

 public:
     PQ9CommandHandler( DataBus &bus, Service **servArray, int count );
     void received( PQ9Frame &newFrame );
     void onValidCommand(void (*function)( void ));
};

#endif /* COMMANDHANDLER_H_ */
