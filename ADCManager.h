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

class ADCManager
{
private:
    static unsigned int enabledADCMem;
    static unsigned int NrOfActiveADC;

public:
    static void initADC();
    static void enableTempMeasurement();
    static uint16_t getTempMeasurement();
    static void executeADC();
};

#endif /* ADCMANAGER_H_ */
