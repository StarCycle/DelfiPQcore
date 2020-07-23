/*
 * FRAMService.h
 *
 *  Created on: 23 Jul 2020
 *      Author: Casper
 */

#ifndef FRAMSERVICE_H_
#define FRAMSERVICE_H_

#include "Service.h"
#include "Console.h"
#include "MB85RS.h"

#define FRAM_SERVICE 20
#define FRAM_SERVICE_INVALID_CMD 1

class FRAMService : public Service
{
private:
    MB85RS* fram;
 public:
    FRAMService(MB85RS& fram_in);
    virtual bool process( DataMessage &command, DataMessage &workingBbuffer );
};


#endif /* FRAMSERVICE_H_ */
