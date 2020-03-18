/*
 * HWMonitor.cpp
 *
 *  Created on: 18 Mar 2020
 *      Author: casperbroekhuizen
 */

#include "HWMonitor.h"
extern DSerial serial;

bool CheckResetSRC(uint32_t Code, uint32_t SRC){
    return ((Code & SRC) == SRC);
}

HWMonitor::HWMonitor(MB85RS* fram_in){
    this->fram = fram_in;
    /* Enabling the FPU with stacking enabled (for use within ISR) */
    MAP_FPU_enableModule();
    MAP_FPU_enableLazyStacking();

    cal30 = MAP_SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,
            SYSCTL_30_DEGREES_C);
    cal85 = MAP_SysCtl_getTempCalibrationConstant(SYSCTL_2_5V_REF,
            SYSCTL_85_DEGREES_C);
    calDifference = cal85 - cal30;
}

void HWMonitor::readMCUTemp(){
    ADCManager::enableTempMeasurement();
}

void HWMonitor::readResetStatus(){
    //Get and Clear ResetRegisters
    serial.println("========== HWMonitor: Reboot Cause ==========");
    this->resetStatus  = (RSTCTL->HARDRESET_STAT   & 0x0F) | ((RSTCTL->HARDRESET_STAT & 0xC000) >> 10);
    this->resetStatus |= (RSTCTL->SOFTRESET_STAT   & 0x07) << 6;
    this->resetStatus |= (RSTCTL->PSSRESET_STAT    & 0x0E) << 8;
    this->resetStatus |= (RSTCTL->PCMRESET_STAT    & 0x03) << 12;
    this->resetStatus |= (RSTCTL->PINRESET_STAT    & 0x01) << 14;
    this->resetStatus |= (RSTCTL->REBOOTRESET_STAT & 0x01) << 15;
    this->resetStatus |= (RSTCTL->CSRESET_STAT     & 0x01) << 16;
    MAP_ResetCtl_clearHardResetSource(((uint32_t) 0x0000FFFF));
    MAP_ResetCtl_clearSoftResetSource(((uint32_t) 0x0000FFFF));
    MAP_ResetCtl_clearPSSFlags();
    MAP_ResetCtl_clearPCMFlags();
    RSTCTL->PINRESET_CLR |= (uint32_t) 0x01;
    RSTCTL->REBOOTRESET_CLR |= (uint32_t) 0x01;
    RSTCTL->CSRESET_CLR |= (uint32_t) 0x01;

    serial.print("RESET STATUS: ");
    serial.print(resetStatus, HEX);
    serial.println("");

    if( CheckResetSRC(resetStatus, RESET_HARD_SYSTEMREQ)){
        serial.println("- POR Caused by System Reset Output of Cortex-M4");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_WDTTIME)){
        serial.println("- POR Caused by HardReset WDT Timer expiration!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_WDTPW_SRC)){
        serial.println("- POR Caused by HardReset WDT Wrong Password!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_FCTL)){
        serial.println("- POR Caused by FCTL detecting a voltage Anomaly!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_CS)){
        serial.println("- POR Extended for Clock Settle!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_PCM) ){
        serial.println("- POR Extended for Power Settle!");
    }
    if( CheckResetSRC(resetStatus, RESET_SOFT_CPULOCKUP) ){
        serial.println("- POR Caused by CPU Lock-up!");
    }
    if( CheckResetSRC(resetStatus, RESET_SOFT_WDTTIME) ){
        serial.println("- POR Caused by SoftReset WDT Timer expiration!!");
    }
    if( CheckResetSRC(resetStatus, RESET_SOFT_WDTPW_SRC) ){
        serial.println("- POR Caused by SoftReset WDT Wrong Password!");
    }
    if( CheckResetSRC(resetStatus, RESET_PSS_VCCDET) ){
        serial.println("- POR Caused by VCC Detector trip condition!");
    }
    if( CheckResetSRC(resetStatus, RESET_PSS_SVSH_TRIP) ){
        serial.println("- POR Caused by Supply Supervisor detected Vcc trip condition!");
    }
    if( CheckResetSRC(resetStatus, RESET_PSS_BGREF_BAD) ){
        serial.println("- POR Caused by Bad Band Gap Reference!");
    }
    if( CheckResetSRC(resetStatus, RESET_PCM_LPM35) ){
        serial.println("- POR Caused by PCM due to exit from LPM3.5!");
    }
    if( CheckResetSRC(resetStatus, RESET_PCM_LPM45) ){
        serial.println("- POR Caused by PCM due to exit from LPM4.5!");
    }
    if( CheckResetSRC(resetStatus, RESET_PIN_NMI) ){
        serial.println("- POR Caused by NMI Pin based event!");
    }
    if( CheckResetSRC(resetStatus, RESET_REBOOT) ){
        serial.println("- POR Caused by SysCTL Reboot!");
    }
    if( CheckResetSRC(resetStatus, RESET_CSRESET_DCOSHORT)){
        serial.println("- POR Caused by DCO short circuit fault in external resistor!");
    }

    if(fram->ping()){
        serial.println("+ FRAM present");
        this->fram->write(FRAM_RESET_CAUSE, &((uint8_t*)&resetStatus)[1], 3);
        uint8_t resetCounter = 0;
        fram->read(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &resetCounter, 1);
        if(!CheckResetSRC(resetStatus, RESET_REBOOT)){
            serial.println("+ Unintentional reset!");
            resetCounter++;
            fram->write(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &resetCounter, 1);
        }else{
            serial.println("+ Intentional reset");
            resetCounter = 0;
            fram->write(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &resetCounter, 1);
        }
        serial.print("+ Reset counter at: ");
        serial.println(resetCounter, DEC);
    }else{
        serial.println("# FRAM unavailable");
    }

    serial.println("=============================================");
}

void HWMonitor::readCSStatus(){
    //Get and clear CLOCK FAULT STATUS
    serial.println("========== HWMonitor: Clock Faults ==========");
    this->CSStatus  = CS->IFG;

    serial.print("CS FAULTS: ");
    serial.print(CSStatus, HEX);
    serial.println("");

    if( CheckResetSRC(CSStatus, CS_IFG_LFXTIFG)){
        serial.println("- Fault in LFXT");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_HFXTIFG)){
        serial.println("- Fault in HFXT");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_DCOR_SHTIFG)){
        serial.println("- DCO Short Circuit!");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_DCOR_OPNIFG)){
        serial.println("- DCO Open Circuit!");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_FCNTLFIFG)){
        serial.println("- LFXT Start-count expired!");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_FCNTHFIFG)){
        serial.println("- HFXT Start-count expired!");
    }

    CS->CLRIFG |= CS_CLRIFG_CLR_LFXTIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_DCOR_OPNIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_FCNTLFIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_FCNTHFIFG;
    CS->CLRIFG |= CS_SETIFG_SET_LFXTIFG;


    serial.println("=============================================");
}

//void HWMonitor::readMCUTemp(){
//    MAP_ADC14_toggleConversionTrigger();
//
//    //wait for measurement
//    while(!(MAP_ADC14_getEnabledInterruptStatus() & ADC_INT22));
//    MAP_ADC14_clearInterruptFlag(ADC_INT22);
//
//    uint16_t conRes = 10 * ((MAP_ADC14_getResult(ADC_MEM0) - cal30) * 55);
//    this->MCUTemp = ((conRes / (10*calDifference)) + 300.0f);
//
//}

uint32_t HWMonitor::getResetStatus(){
    return resetStatus;
}

uint32_t HWMonitor::getCSStatus(){
    return CSStatus;
}

uint16_t HWMonitor::getMCUTemp(){
    uint16_t conRes = 10 * ((ADCManager::getTempMeasurement() - cal30) * 55);
    this->MCUTemp = ((conRes / (10*calDifference)) + 300.0f);

    return MCUTemp;
}
