/*
 * CodeCoverageService.h
 *
 *  Created on: 24 Jan 2022
 *      Author: Zhuoheng Li
 */

#ifndef CODECOVERAGESERVICE_H_
#define CODECOVERAGESERVICE_H_

#include "Service.h"
#include "CodeCoverageArray.h"

#define CODECOVERAGE_SERVICE    97
#define MAX_PAYLOAD_SIZE        253

class CodeCoverageService: public Service
{

 public:
    bool process( DataMessage &command, DataMessage &workingBuffer )
    {
        if (command.getService() == CODECOVERAGE_SERVICE)
        {
            Console::log("CodeCoverageService: Request");
            workingBuffer.setService(CODECOVERAGE_SERVICE);
            workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);

            switch(command.getDataPayload()[0]){
            case 0:
                // simply retrieve code coverage
                for (int i = 0; i < MAX_PAYLOAD_SIZE; i++)
                {
                    workingBuffer.getDataPayload()[i] = coverageArray[i];
                }
                workingBuffer.setPayloadSize(MAX_PAYLOAD_SIZE);
                break;

            case 1:
                // retrieve transitions
                char region = command.getDataPayload()[1];  // range of region: 0~11
                for (int i = 0; i < MAX_PAYLOAD_SIZE; i++)
                {
                    workingBuffer.getDataPayload()[i] = ((unsigned char *)lastTransition)[i + region*MAX_PAYLOAD_SIZE];
                }
                workingBuffer.setPayloadSize(MAX_PAYLOAD_SIZE);
            }
            return true;
        }
        else
        {
            // this command is related to another service,
            // report the command was not processed
            return false;
        }
    }

};

#endif /*CODECOVERAGESERVICE_H_ */
