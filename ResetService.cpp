/*
 * ResetService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#include "ResetService.h"

extern DSerial serial;
ResetService* resetServiceStub;
void _forceHardReset(){
    resetServiceStub->forceHardReset();
}
void _forceSoftReset(){
    resetServiceStub->forceSoftReset();
}


bool CheckResetSRC(uint32_t Code, uint32_t SRC){
    return ((Code & SRC) == SRC);
}

/**
 *
 *   Reset device, triggered by WatchDog interrupt (time-out)
 *
 *   Parameters:

 *   Returns:
 *
 */
void resetHandler()
{
    serial.println("ResetService: internal watch-dog reset... ");
    serial.println("");
    //Add WDT time=out to reset-cause register
    RSTCTL->HARDRESET_SET |= RESET_HARD_WDTTIME;
    // TODO: flush the serial port to make sure all characters have been trinted out before resetting
    uint32_t d = MAP_CS_getMCLK() * 4 / 9600;
    for(uint32_t k = 0; k < d;  k++)
    {
        __asm("  nop");
    }
    // TODO: replace this with a power cycle to protect also the RS485 driver
    // for now, at least reset, till the power cycle gets implemented in HW
    // MAP_SysCtl_rebootDevice();
    MAP_ResetCtl_initiateHardReset();
}

/**
 *
 *   ResetService Constructor
 *
 *   Parameters:
 *   WDport                     External watch-dog reset port
 *   WDpin                      External watch-dog reset pin
 *
 *   Returns:
 *
 */
ResetService::ResetService(const unsigned long WDport, const unsigned long WDpin) :
        WDIPort(WDport), WDIPin(WDpin) {
    resetServiceStub = this;
}

/**
 *
 *   ResetService Initialize watch-dog service and interrupts.
 *
 *   Parameters:

 *   Returns:
 *
 */
void ResetService::init()
{
    // initialize the internal watch-dog
    MAP_WDT_A_clearTimer();                                  // Clear the watch-dog to prevent spurious triggers
    MAP_WDT_A_initIntervalTimer( WDT_A_CLOCKSOURCE_SMCLK,    // set the watch-dog to trigger every 178s
                                 WDT_A_CLOCKITERATIONS_2G ); // (about 3 minutes)

    // select the interrupt handler
    MAP_WDT_A_registerInterrupt(&resetHandler);

    // initialize external watch-dog pins
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setAsOutputPin( WDIPort, WDIPin );

    // start the timer
    MAP_WDT_A_startTimer();

    //get ResetStatus
    readResetStatus();

}

void ResetService::readResetStatus(){

    //Get and Clear ResetRegisters
    serial.println("========= ResetService Reboot Cause =========");
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
    serial.println("=============================================");

//    this->softResetStat = MAP_ResetCtl_getSoftResetSource();
//    serial.print("SOFT RESET: ");
//    serial.print(this->softResetStat, HEX);
//    serial.println("");
//    MAP_ResetCtl_clearSoftResetSource(((uint32_t) 0x0000FFFF));
//    if(softResetStat != 0){
//        if((softResetStat & RESET_SOFT_CPULOCKUP) == RESET_SOFT_CPULOCKUP){
//            serial.println("- POR Caused by CPU Lock-up!");
//        }
//        if((softResetStat & RESET_SOFT_WDTTIME) == RESET_SOFT_WDTTIME){
//            serial.println("- POR Caused by SoftReset WDT Timer expiration!!");
//        }
//        if((softResetStat & RESET_SOFT_WDTPW_SRC) == RESET_SOFT_WDTPW_SRC){
//            serial.println("- POR Caused by SoftReset WDT Wrong Password!");
//        }
//    }
//
//    this->hardResetStat = MAP_ResetCtl_getHardResetSource();
//    serial.print("HARD RESET: ");
//    serial.print(this->hardResetStat, HEX);
//    serial.println("");
//    MAP_ResetCtl_clearHardResetSource(((uint32_t) 0x0000FFFF));
//    if(hardResetStat != 0){
//        if( (hardResetStat & RESET_HARD_SYSTEMREQ) == RESET_HARD_SYSTEMREQ){
//            serial.println("- POR Caused by System Reset Output of Cortex-M4");
//        }
//        if( (hardResetStat & RESET_HARD_WDTTIME) == RESET_HARD_WDTTIME){
//            serial.println("- POR Caused by HardReset WDT Timer expiration!");
//        }
//        if( (hardResetStat & RESET_HARD_WDTPW_SRC) == RESET_HARD_WDTPW_SRC){
//            serial.println("- POR Caused by HardReset WDT Wrong Password!");
//        }
//        if( (hardResetStat & RESET_HARD_FCTL) == RESET_HARD_FCTL){
//            serial.println("- POR Caused by FCTL detecting a voltage Anomaly!");
//        }
//        if( (hardResetStat & RESET_HARD_CS) == RESET_HARD_CS){
//            serial.println("- POR Extended for Clock Settle!");
//        }
//        if( (hardResetStat & RESET_HARD_PCM) == RESET_HARD_PCM){
//            serial.println("- POR Extended for Power Settle!");
//        }
//    }
//
//    this->pssResetStat = MAP_ResetCtl_getPSSSource();
//    serial.print("PSS RESET: ");
//    serial.print(this->pssResetStat, HEX);
//    serial.println("");
//    MAP_ResetCtl_clearPSSFlags();
//    if(pssResetStat != 0){
//        if((pssResetStat & RESET_PSS_VCCDET) == RESET_PSS_VCCDET){
//            serial.println("- POR Caused by VCC Detector trip condition!");
//        }
//        if((pssResetStat & RESET_PSS_SVSH_TRIP) == RESET_PSS_SVSH_TRIP){
//            serial.println("- POR Caused by Supply Supervisor detected Vcc trip condition!");
//        }
//        if((pssResetStat & RESET_PSS_BGREF_BAD) == RESET_PSS_BGREF_BAD){
//            serial.println("- POR Caused by Bad Band Gap Reference!");
//        }
//    }
//
//    this->pcmResetStat = MAP_ResetCtl_getPCMSource();
//    MAP_ResetCtl_clearPCMFlags();
//    serial.print("PCM RESET: ");
//    serial.print(this->pcmResetStat, HEX);
//    serial.println("");
//    if(pcmResetStat != 0){
//        if((pcmResetStat & RESET_PCM_LPM35) == RESET_PCM_LPM35){
//            serial.println("- POR Caused by PCM due to exit from LPM3.5!");
//        }
//        if((pcmResetStat & RESET_PCM_LPM45) == RESET_PCM_LPM45){
//            serial.println("- POR Caused by PCM due to exit from LPM4.5!");
//        }
//    }
//
//    this->pinResetStat = RSTCTL->PINRESET_STAT;
//    RSTCTL->PINRESET_CLR |= (uint32_t) 0x01;
//    serial.print("PIN RESET: ");
//    serial.print(this->pinResetStat, HEX);
//    serial.println("");
//    if(pinResetStat != 0){
//        if((pinResetStat & RESET_PIN_NMI) == RESET_PIN_NMI){
//            serial.println("- POR Caused by NMI Pin based event!");
//        }
//    }
//
//    this->rebootResetStat = RSTCTL->REBOOTRESET_STAT;
//    RSTCTL->REBOOTRESET_STAT |= (uint32_t) 0x01;
//    serial.print("REBOOT RESET: ");
//    serial.print(this->rebootResetStat, HEX);
//    serial.println("");
//    if(rebootResetStat != 0){
//        if((rebootResetStat & RESET_REBOOT) == RESET_REBOOT){
//            serial.println("- POR Caused by SysCTL Reboot!");
//        }
//    }
//
//    this->csResetStat = RSTCTL->CSRESET_STAT;
//    RSTCTL->CSRESET_STAT |= (uint32_t) 0x01;
//    serial.print("CS RESET: ");
//    serial.print(this->csResetStat, HEX);
//    serial.println("");
//    if(csResetStat != 0){
//        if((csResetStat & RESET_CSRESET_DCOSHORT) == RESET_CSRESET_DCOSHORT){
//            serial.println("- POR Caused by DCO short circuit fault in external resistor!");
//        }
//    }
//    serial.println("=============================================");
}

/**
 *
 *   Reset internal watch-dog interrupt and timer and set external watch-dog port/pins as output.
 *
 *   Parameters:

 *   Returns:
 *
 */
void ResetService::refreshConfiguration()
{
    // select the interrupt handler
    MAP_WDT_A_registerInterrupt(&resetHandler);

    // ensure the timer is running: this only forces the
    // timer to run (in case it got disabled for any reason)
    // but it does not reset it, making sure the watch-dog
    // cannot be disabled by mistake
    MAP_WDT_A_startTimer();

    // initialize external watch-dog pins
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setAsOutputPin( WDIPort, WDIPin );
}

/**
 *
 *   Kick internal watch-dog by resetting timer, should be called every (3min) or reset.
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::kickInternalWatchDog()
{
    // reset the internal watch-dog timer
    MAP_WDT_A_clearTimer();
}

/**
 *
 *   Kick external watch-dog by resetting timer.
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::kickExternalWatchDog()
{
    // toggle the WDI pin of the external watch-dog
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
}

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   DataBus &interface       Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */
bool ResetService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getPayload()[0] == RESET_SERVICE)
    {
        serial.print("ResetService: ");
        // prepare response frame
        //workingBuffer.setDestination(command.getSource());
        //workingBuffer.setSource(interface.getAddress());
        workingBuffer.setSize(3);
        workingBuffer.getPayload()[0] = RESET_SERVICE;

        if (command.getPayload()[1] == SERVICE_RESPONSE_REQUEST)
        {
            serial.print("ResetRequest: ");
            workingBuffer.getPayload()[2] = command.getPayload()[2];
            switch(command.getPayload()[2])
            {
                case RESET_SOFT:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    //interface.transmit(workingBuffer);

                    // now reset the MCU
                    serial.println("SofReset set?");
                    this->setPostFunc(_forceSoftReset);
                    break;

                case RESET_HARD:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    //interface.transmit(workingBuffer);

                    // now force the external watch-dog to reset the MCU
                    this->setPostFunc(_forceHardReset);
                    break;

                    // not implemented yet, give error to notify it
                /*case RESET_POWERCYCLE:
                    serial.println("POWERCYCLE");
                    workingBuffer.getPayload()[1] = RESET_RESPONSE;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    interface.transmit(workingBuffer);
                    break;*/

                default:
                    workingBuffer.getPayload()[1] = SERVICE_RESPONSE_ERROR;
                    // send response: doing it here to make sure
                    // a response is sent before reset but not 2
                    //interface.transmit(workingBuffer);
                    break;
            }
        }
        else
        {
            // unknown request
            workingBuffer.getPayload()[1] = RESET_ERROR;
            // send response: doing it here to make sure
            // a response is sent before reset but not 2
            //interface.transmit(workingBuffer);
        }

        // command processed
        return true;
    }
    else
    {
        // this command is related to another service,
        // report the command was not processed
        return false;
    }
}

/**
 *
 *   Force the external watch-dog to reset the MCU
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::forceHardReset()
{
    serial.println("ResetService: Hard reset");
    // TODO: flush the serial port to make sure all characters have been trinted out before resetting
    uint32_t d = MAP_CS_getMCLK() * 4 / 9600;
    for(uint32_t k = 0; k < d;  k++)
    {
        __asm("  nop");
    }
    // toggle the WDI pin 3 times, just to be sure
    // the external watch-dog resets...
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputHighOnPin( WDIPort, WDIPin );
    MAP_GPIO_setOutputLowOnPin( WDIPort, WDIPin );
}

/**
 *
 *   Force the internal watch-dog to reset the MCU
 *
 *   Parameters:
 *
 *   Returns:
 *
 */
void ResetService::forceSoftReset()
{
    serial.println("ResetService: Soft reset");
    // TODO: flush the serial port to make sure all characters have been trinted out before resetting
    uint32_t d = MAP_CS_getMCLK() * 4 / 9600;
    for(uint32_t k = 0; k < d;  k++)
    {
        __asm("  nop");
    }
    MAP_SysCtl_rebootDevice();
}

uint32_t ResetService::getResetStatus(){
    return this->resetStatus;
}
