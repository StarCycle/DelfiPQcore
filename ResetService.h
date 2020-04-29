/*
 * ResetService.h
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#ifndef RESETSERVICE_H_
#define RESETSERVICE_H_

#include "Service.h"
#include "Console.h"
#include "ResetCodes.h"
#include "MB85RS.h"
#include "FRAMMap.h"
#include "Bootloader.h"

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
     const unsigned long ERPort;
     const unsigned long ERPin;

 public:
     ResetService( const unsigned long port, const unsigned long pin );
     ResetService( const unsigned long port, const unsigned long pin, const unsigned long Rport, const unsigned long Rpin );
     virtual bool process( DataMessage &command, DataMessage &workingBbuffer );
     void init();

     void refreshConfiguration();
     void kickExternalWatchDog();
     void kickInternalWatchDog();

     void forcePowerCycle();
     void forceHardReset();
     void forceSoftReset();
};

#endif /* RESETSERVICE_H_ */
