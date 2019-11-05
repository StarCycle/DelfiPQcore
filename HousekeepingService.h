/*
 * TelemetryService.h
 *
 *  Created on: 4 Aug 2019
 *      Author: stefanosperett
 */

#ifndef HOUSEKEEPINGSERVICE_H_
#define HOUSEKEEPINGSERVICE_H_

#include "PQ9Frame.h"
#include "Service.h"
#include "TelemetryContainer.h"
#include "DSerial.h"

#define HOUSEKEEPING_SERVICE             3
#define HOUSEKEEPING_ERROR               0
#define HOUSEKEEPING_REQUEST             1
#define HOUSEKEEPING_RESPONSE            2

template <class tlmType>
class HousekeepingService: public Service
{
protected:
    int telemetryIndex = 0;
    tlmType telemetryContainer[2];

    void stageTelemetry();

 public:
    HousekeepingService();
    virtual ~HousekeepingService();
    bool process( PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBbuffer );
    tlmType* getContainerToRead();
    tlmType* getContainerToWrite();

};

#endif /* HOUSEKEEPINGSERVICE_H_ */
