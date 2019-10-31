/*
 * PQ9CommandHandler.h
 *
 *  Created on: 24 Jul 2019
 *      Author: stefanosperett
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "PQ9Frame.h"
#include "Service.h"
#include "Task.h"
#include "PingService.h"
#include "DSerial.h"

class PQ9CommandHandler: public Task
{
 protected:
     //PQ9Bus &bus;
     //Service** services;
     //int servicesCount;

     //void (*onValidCmd)( void );
     //friend void stubCommandHandler( );

 public:
     PQ9CommandHandler( PQ9Bus &bus, Service **servArray, int count );
     void received( PQ9Frame &newFrame );
     void onValidCommand(void (*function)( void ));
     void (*onValidCmd)( void );
     bool handleCommands();
};

#endif /* COMMANDHANDLER_H_ */
