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

const uint32_t ADCManager::MemoryLocations[32] = {ADC_MEM0 ,ADC_MEM1 ,ADC_MEM2 ,ADC_MEM3 ,ADC_MEM4 ,ADC_MEM5 ,ADC_MEM6 ,ADC_MEM7 ,ADC_MEM8 ,ADC_MEM9 ,ADC_MEM10,
                                ADC_MEM11,ADC_MEM12,ADC_MEM13,ADC_MEM14,ADC_MEM15,ADC_MEM16,ADC_MEM17,ADC_MEM18,ADC_MEM19,ADC_MEM20,ADC_MEM21,
                                ADC_MEM22,ADC_MEM23,ADC_MEM24,ADC_MEM25,ADC_MEM26,ADC_MEM27,ADC_MEM28,ADC_MEM29,ADC_MEM30,ADC_MEM31};

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
    //Enabling the FPU with stacking enabled (for use within ISR)
    MAP_FPU_enableModule();
    MAP_FPU_enableLazyStacking();

    /* Setting reference voltage to 2.5 and enabling temperature sensor */
    MAP_REF_A_enableTempSensor();
    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    ADCManager::enableTempMeasurement();
}

void ADCManager::enableTempMeasurement(){
//    /* Configuring ADC Memory (ADC_MEM22 A22 (Temperature Sensor) in repeat ) */
    if(!(enabledADCMem & 0x01)){
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
    if(NrOfActiveADC == 0){
        return; //no active ADC
    }else if(NrOfActiveADC == 1){
        MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true); //only MEM0 is active
    }else if(NrOfActiveADC < 32){
        //NrOfActive > 0 but not 1
        MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, MemoryLocations[NrOfActiveADC-1], true);
    }else{
        //should never happen
        serial.println("ADCManager: impossible configuration!");
        return;
    }
    /* Triggering the start of the sample */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();
}


/*
 * registerADC()
 * parameter: (ADC Pin number)
 * returns: ADC Mem Location
 *
 * Description:
 *  Register an Analog Pin to the ADC to be measured continuously.
 *  If a ADC 'slot' is available, the Manager will return a number with the Memory location,
 *  which can be used to obtain the measured result using getMeasurement(int memNumber)
 */
int ADCManager::registerADC(unsigned long ADCPin){
    if((NrOfActiveADC < 32) && (NrOfActiveADC > 0)){ //ADC Slot Available
        int newRegisteredMem = NrOfActiveADC;
        MAP_ADC14_configureConversionMemory(MemoryLocations[newRegisteredMem], ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADCPin, false);

        enabledADCMem |= (1 << newRegisteredMem);
        NrOfActiveADC += 1;
        ADCManager::executeADC();

        return newRegisteredMem;
    } else {
        return -1;
    }
}

/*
 * getMeasurementRaw()
 * parameter: (MEM address number)
 * returns: ADC measurement [in bits]
 *
 * Description:
 *  Get the latest measurement from the ADCManager of the selected Memory
 *
 */
uint16_t ADCManager::getMeasurementRaw(int memNumber){
    return MAP_ADC14_getResult(MemoryLocations[memNumber]);
}

/*
 * getMeasurementRaw()
 * parameter: (MEM address number)
 * returns: ADC measurement [in mV]
 *
 * Description:
 *  Get the latest measurement from the ADCManager of the selected Memory
 *
 */
uint16_t ADCManager::getMeasurementVolt(int memNumber){
    return (MAP_ADC14_getResult(MemoryLocations[memNumber]) * 2500 / 16384);
}




