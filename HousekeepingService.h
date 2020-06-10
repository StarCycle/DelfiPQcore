/*
 * TelemetryService.h
 *
 *  Created on: 4 Aug 2019
 *      Author: stefanosperett
 */

#ifndef HOUSEKEEPINGSERVICE_H_
#define HOUSEKEEPINGSERVICE_H_

#include <type_traits>
#include "Service.h"
#include "TelemetryContainer.h"
#include "Console.h"

#define HOUSEKEEPING_SERVICE             3
#define HOUSEKEEPING_ERROR               0
#define HOUSEKEEPING_REQUEST             1
#define HOUSEKEEPING_RESPONSE            2

template <class T>
class HousekeepingService: public Service
{
    // only allow types derived from TelemetryContainer
    //static_assert(std::is_base_of<TelemetryContainer, T>::value, "HousekeepingService template type is not derived from TelemetryContainer");
private:
    void stageTelemetry()
    {
        // swap the pointers to the storage / readout entities
        telemetryIndex++;
        telemetryIndex %= 2;
    }
    
    T* getContainerToWrite()
    {
        // return the container not currently used to read the telemetry
        return &(telemetryContainer[telemetryIndex]);
    }

protected:
    int telemetryIndex = 0;
    T telemetryContainer[2];

 public:
    bool process( DataMessage &command, DataMessage &workingBuffer )
    {
        if (command.getService() == HOUSEKEEPING_SERVICE)
        {
            // prepare response frame
            //workingBuffer.setDestination(command.getSource());
            //workingBuffer.setSource(interface.getAddress());
            workingBuffer.setService(HOUSEKEEPING_SERVICE);

            Console::log("HousekeepingService: Request");
            // respond to housekeeping request
            workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);

            for (int i = 0; i < getTelemetry()->size(); i++)
            {
                workingBuffer.getDataPayload()[i] = getTelemetry()->getArray()[i];
            }
            workingBuffer.setPayloadSize(getTelemetry()->size());

            return true;
        }
        else
        {
            // this command is related to another service,
            // report the command was not processed
            return false;
        }
    }

    T* getTelemetry()
    {
        return &(telemetryContainer[(telemetryIndex + 1) % 2]);
    }

    void acquireTelemetry(void (*callback)( T* ))
    {
        // acquire telemetry via the callback
        callback(getContainerToWrite());

        // telemetry collected, store the values and prepare for next collection
        stageTelemetry();
    }
};

#endif /* HOUSEKEEPINGSERVICE_H_ */
