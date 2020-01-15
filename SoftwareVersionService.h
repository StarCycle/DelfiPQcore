/*
 * SoftwareVersionService.h
 *
 *  Created on: 02 Dec 2019
 *      Author: Casper Broekhuizen
 */

#ifndef SOFTWAREVERSIONSERVICE_H_
#define SOFTWAREVERSIONSERVICE_H_

#include "PQ9Bus.h"
#include "PQ9Frame.h"
#include "Service.h"
#include "DSerial.h"

#define xstr(s) str(s)
#define str(s) #s

#define SOFTWAREVERSION_SERVICE         23

#define SOFTWAREVERSION_GETSOFTWAREVERSION       1
#define SOFTWAREVERSION_ACCEPT          2
#define SOFTWAREVERSION_ERROR           0

class SoftwareVersionService: public Service
{
 public:
    SoftwareVersionService();
    SoftwareVersionService(uint8_t versionNumberIn[]);
    virtual bool process( DataFrame &command, DataBus &interface, DataFrame &workingBbuffer );

private:
    bool hasVersionNumber;
    uint8_t versionNumber[8] = {0};
};

#endif /* SOFTWAREVERSIONSERVICE_H_ */
