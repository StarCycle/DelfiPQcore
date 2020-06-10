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

#define RESET_SERVICE            19

#define ERROR_NO_ERROR           0
#define ERROR_UNKNOWN_COMMAND    1

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
     virtual bool process( DataMessage &command, DataMessage &workingBbuffer );
     void init();

     void refreshConfiguration();
     void kickExternalWatchDog();
     void kickInternalWatchDog();

     void forceHardReset();
     void forceSoftReset();
};

#endif /* RESETSERVICE_H_ */
