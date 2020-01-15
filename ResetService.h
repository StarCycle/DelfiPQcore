/*
 * ResetService.h
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#ifndef RESETSERVICE_H_
#define RESETSERVICE_H_

#include "Service.h"
#include "DSerial.h"

#define RESET_SERVICE           19
#define RESET_ERROR              0
#define RESET_REQUEST            1
#define RESET_RESPONSE           2

#define RESET_SOFT               1
#define RESET_HARD               2
#define RESET_POWERCYCLE         3

class ResetService: public Service
{
 protected:
     const unsigned long WDIPort;
     const unsigned long WDIPin;

 public:
     ResetService( const unsigned long port, const unsigned long pin );
     virtual bool process( DataFrame &command, DataBus &interface, DataFrame &workingBbuffer );
     void init();
     void refreshConfiguration();
     void kickExternalWatchDog();
     void kickInternalWatchDog();

     void forceHardReset();
     void forceSoftReset();
};

#endif /* RESETSERVICE_H_ */
