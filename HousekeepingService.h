/*
 * TelemetryService.h
 *
 *  Created on: 4 Aug 2019
 *      Author: stefanosperett
 */

#ifndef HOUSEKEEPINGSERVICE_H_
#define HOUSEKEEPINGSERVICE_H_

#include <type_traits>
#include "PQ9Frame.h"
#include "Service.h"
#include "TelemetryContainer.h"
#include "DSerial.h"

#define HOUSEKEEPING_SERVICE             3
#define HOUSEKEEPING_ERROR               0
#define HOUSEKEEPING_REQUEST             1
#define HOUSEKEEPING_RESPONSE            2

extern DSerial serial;

template <class T>
class HousekeepingService: public Service
{
    // only allow types derived from TelemetryContainer
    static_assert(std::is_base_of<TelemetryContainer, T>::value, "HousekeepingService template type is not derived from TelemetryContainer");
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
    bool process( PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBuffer )
    {
        if (command.getPayload()[0] == HOUSEKEEPING_SERVICE)
        {
            // prepare response frame
            workingBuffer.setDestination(command.getSource());
            workingBuffer.setSource(interface.getAddress());
            workingBuffer.getPayload()[0] = HOUSEKEEPING_SERVICE;

            if (command.getPayload()[1] == HOUSEKEEPING_REQUEST)
            {
                serial.println("HousekeepingService: Request");

                // respond to housekeeping request
                workingBuffer.getPayload()[1] = HOUSEKEEPING_RESPONSE;
                for (int i = 0; i < getTelemetry()->size(); i++)
                {
                    workingBuffer.getPayload()[i + 2] = getTelemetry()->getArray()[i];
                }
                workingBuffer.setPayloadSize(2 + getTelemetry()->size());
            }
            else
            {
                // unknown request
                workingBuffer.getPayload()[1] = HOUSEKEEPING_ERROR;
                workingBuffer.setPayloadSize(2);
            }

            // send response
            interface.transmit(workingBuffer);
            // command processed
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
