/*
 * ADC14.h
 *
 *  Created on: 18 Mar 2020
 *      Author: casperbroekhuizen
 *
 *  Wrapper class for ADC14 Module
 */

#ifndef ADCMANAGER_H_
#define ADCMANAGER_H_

#include <driverlib.h>
#include "msp.h"
#include "Console.h"

class ADCManager
{
private:

    ADCManager();
    static unsigned int enabledADCMem;
    static unsigned int NrOfActiveADC;
    static const uint32_t MemoryLocations[32];

public:
    static void initADC();
    static void enableTempMeasurement();
    static float getTempMeasurement();
    static void executeADC();
    static int registerADC(const unsigned long ADCPin);
    static uint16_t getMeasurementRaw(int memNumber);
    static uint16_t getMeasurementVolt(int memNumber);
};

#endif /* ADCMANAGER_H_ */
