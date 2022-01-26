/*
 * CodeCoverageService.h
 *
 *  Created on: 24 Jan 2022
 *      Author: Zhuoheng Li
 */

#ifndef CODECOVERAGESERVICE_H_
#define CODECOVERAGESERVICE_H_

#include "CodeCoverageArray.h"
#include "Service.h"

#define CODECOVERAGE_SERVICE    97

class CodeCoverageService: public Service
{

 public:
    bool process( DataMessage &command, DataMessage &workingBuffer )
    {
        if (command.getService() == CODECOVERAGE_SERVICE)
        {
            workingBuffer.setService(CODECOVERAGE_SERVICE);
            Console::log("CodeCoverageService: Request");
            workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);
            for (int i = 0; i < 253; i++)
            {
                workingBuffer.getDataPayload()[i] = coverageArray[i];
                coverageArray[i] = 0; // Reset the array
            }
            workingBuffer.setPayloadSize(253);
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
