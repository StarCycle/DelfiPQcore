/*
 * ADC14.cpp
 *
 *  Created on: 18 Mar 2020
 *      Author: casperbroekhuizen
 */

#include "ADCManager.h"
#include "DSerial.h"

extern DSerial serial;

unsigned int ADCManager::enabledADCMem = 0;
unsigned int ADCManager::NrOfActiveADC = 0;

void ADCManager::initADC(){
    /* Setting reference voltage to 2.5  and enabling reference */
    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();
    MAP_REF_A_enableTempSensor();

    /* Initializing ADC (MCLK/1/1) with internal TEMPSENSEMAP */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_64, ADC_DIVIDER_8, ADC_TEMPSENSEMAP);

    /* Configuring ADC Memory (ADC_MEM22 A22 (Temperature Sensor) in repeat ) */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    /* Set Sample hold-time */
    MAP_ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192,ADC_PULSE_WIDTH_192);

    /* Enabling sample timer in auto iteration mode and interrupts*/
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

}

void ADCManager::enableTempMeasurement(){
    /* Configuring ADC Memory (ADC_MEM22 A22 (Temperature Sensor) in repeat ) */
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A22, false);
    enabledADCMem |= 1;
    NrOfActiveADC += 1;
    ADCManager::executeADC();
}

uint16_t ADCManager::getTempMeasurement(){
    return MAP_ADC14_getResult(ADC_MEM0);
}

void ADCManager::executeADC(){
    MAP_ADC14_disableConversion();
    switch(NrOfActiveADC){
        case 0:
            return;
            break;
        case 1:
            MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
            break;
        case 2:
            MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
            break;
    }
    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();
}


