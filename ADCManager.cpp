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
    /* Initializing ADC (MCLK/1/1) with temperature sensor routed */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,
            ADC_TEMPSENSEMAP);

    /* Configuring the sample/hold time for 192 */
    MAP_ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_192,ADC_PULSE_WIDTH_192);

    /* Enabling sample timer in auto iteration mode and interrupts*/
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
    //ADC14_enableInterrupt(ADC_INT0);
}

void ADCManager::enableTempMeasurement(){
//    /* Configuring ADC Memory (ADC_MEM22 A22 (Temperature Sensor) in repeat ) */
    if(!(enabledADCMem & 0x01)){
        //Enabling the FPU with stacking enabled (for use within ISR)
        MAP_FPU_enableModule();
        MAP_FPU_enableLazyStacking();

        /* Setting reference voltage to 2.5 and enabling temperature sensor */
        MAP_REF_A_enableTempSensor();
        MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
        MAP_REF_A_enableReferenceVoltage();
        MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A22, false);
        enabledADCMem |= 1;
        NrOfActiveADC += 1;
        ADCManager::executeADC();
    }
}

float ADCManager::getTempMeasurement(){
    if((enabledADCMem & 0x01)){
        uint32_t cal30;
        uint32_t cal85;
        float calDifference;
        float tempC;
        int16_t conRes;
        uint64_t status;

        cal30 = MAP_SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,
                SYSCTL_30_DEGREES_C);
        cal85 = MAP_SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,
                SYSCTL_85_DEGREES_C);
        calDifference = cal85 - cal30;

        conRes = ((MAP_ADC14_getResult(ADC_MEM0) - cal30) * 55);
        tempC = (conRes / calDifference) + 30.0f;
        return tempC;
    }else{
        return 0;
    }
}

void ADCManager::executeADC(){
    MAP_ADC14_disableConversion();
    switch(NrOfActiveADC){
        case 0:
            return;
            break;
        case 1:
            //Enable multiSampling mode+
            MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
            break;
        case 2:
            MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
            serial.print("ADCManager: Case 2+ not implemented, Should not happen!");
            break;
        default:
            serial.print("ADCManager: Default Case should not happen!");
            break;
    }
    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();
}


