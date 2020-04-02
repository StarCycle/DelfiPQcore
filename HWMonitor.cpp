/*
 * HWMonitor.cpp
 *
 *  Created on: 18 Mar 2020
 *      Author: casperbroekhuizen
 */

#include "HWMonitor.h"

bool CheckResetSRC(uint32_t Code, uint32_t SRC){
    return ((Code & SRC) == SRC);
}

HWMonitor::HWMonitor(MB85RS* fram_in){
    this->fram = fram_in;
}

void HWMonitor::readResetStatus(){
    //Get and Clear ResetRegisters
    Console::log("========== HWMonitor: Reboot Cause ==========");
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

    Console::log("RESET STATUS: %x", resetStatus);

    if( CheckResetSRC(resetStatus, RESET_HARD_SYSTEMREQ)){
        Console::log("- POR Caused by System Reset Output of Cortex-M4");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_WDTTIME)){
        Console::log("- POR Caused by HardReset WDT Timer expiration!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_WDTPW_SRC)){
        Console::log("- POR Caused by HardReset WDT Wrong Password!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_FCTL)){
        Console::log("- POR Caused by FCTL detecting a voltage Anomaly!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_CS)){
        Console::log("- POR Extended for Clock Settle!");
    }
    if( CheckResetSRC(resetStatus, RESET_HARD_PCM) ){
        Console::log("- POR Extended for Power Settle!");
    }
    if( CheckResetSRC(resetStatus, RESET_SOFT_CPULOCKUP) ){
        Console::log("- POR Caused by CPU Lock-up!");
    }
    if( CheckResetSRC(resetStatus, RESET_SOFT_WDTTIME) ){
        Console::log("- POR Caused by SoftReset WDT Timer expiration!!");
    }
    if( CheckResetSRC(resetStatus, RESET_SOFT_WDTPW_SRC) ){
        Console::log("- POR Caused by SoftReset WDT Wrong Password!");
    }
    if( CheckResetSRC(resetStatus, RESET_PSS_VCCDET) ){
        Console::log("- POR Caused by VCC Detector trip condition!");
    }
    if( CheckResetSRC(resetStatus, RESET_PSS_SVSH_TRIP) ){
        Console::log("- POR Caused by Supply Supervisor detected Vcc trip condition!");
    }
    if( CheckResetSRC(resetStatus, RESET_PSS_BGREF_BAD) ){
        Console::log("- POR Caused by Bad Band Gap Reference!");
    }
    if( CheckResetSRC(resetStatus, RESET_PCM_LPM35) ){
        Console::log("- POR Caused by PCM due to exit from LPM3.5!");
    }
    if( CheckResetSRC(resetStatus, RESET_PCM_LPM45) ){
        Console::log("- POR Caused by PCM due to exit from LPM4.5!");
    }
    if( CheckResetSRC(resetStatus, RESET_PIN_NMI) ){
        Console::log("- POR Caused by NMI Pin based event!");
    }
    if( CheckResetSRC(resetStatus, RESET_REBOOT) ){
        Console::log("- POR Caused by SysCTL Reboot!");
    }
    if( CheckResetSRC(resetStatus, RESET_CSRESET_DCOSHORT)){
        Console::log("- POR Caused by DCO short circuit fault in external resistor!");
    }

    if(fram->ping()){
        Console::log("+ FRAM present");
        this->fram->write(FRAM_RESET_CAUSE, &((uint8_t*)&resetStatus)[1], 3);
        uint8_t resetCounter = 0;
        Console::log("+ Current Slot: %d", (int) Bootloader::getCurrentSlot());
        fram->read(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &resetCounter, 1);
        if(!CheckResetSRC(resetStatus, RESET_REBOOT)){
            Console::log("+ Unintentional reset!");
            resetCounter++;
            fram->write(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &resetCounter, 1);
        }else{
            Console::log("+ Intentional reset");
            resetCounter = 0;
            fram->write(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &resetCounter, 1);
        }
        Console::log("+ Reset counter at: %d", (int) resetCounter);
    }else{
        Console::log("# FRAM unavailable");
    }

    Console::log("=============================================");
}

void HWMonitor::readCSStatus(){
    //Get and clear CLOCK FAULT STATUS
    Console::log("========== HWMonitor: Clock Faults ==========");
    this->CSStatus  = CS->IFG;

    Console::log("CS FAULTS: %x", CSStatus);

    if( CheckResetSRC(CSStatus, CS_IFG_LFXTIFG)){
        Console::log("- Fault in LFXT");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_HFXTIFG)){
        Console::log("- Fault in HFXT");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_DCOR_SHTIFG)){
        Console::log("- DCO Short Circuit!");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_DCOR_OPNIFG)){
        Console::log("- DCO Open Circuit!");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_FCNTLFIFG)){
        Console::log("- LFXT Start-count expired!");
    }
    if( CheckResetSRC(CSStatus, CS_IFG_FCNTHFIFG)){
        Console::log("- HFXT Start-count expired!");
    }

    CS->CLRIFG |= CS_CLRIFG_CLR_LFXTIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_DCOR_OPNIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_FCNTLFIFG;
    CS->CLRIFG |= CS_CLRIFG_CLR_FCNTHFIFG;
    CS->CLRIFG |= CS_SETIFG_SET_LFXTIFG;


    Console::log("=============================================");
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
    this->MCUTemp = 10*ADCManager::getTempMeasurement();

    return MCUTemp;
}
