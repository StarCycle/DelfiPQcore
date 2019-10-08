/*
 * DelfiPQcore.cpp
 *
 *  Created on: 26 Aug 2019
 *      Author: stefanosperett
 */

#include <DelfiPQcore.h>

void DelfiPQcore::initMCU()
{
    // Configuring pins for HF XTAL
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    // Starting HFXT in non-bypass mode without a timeout. Before we start
    // we have to change VCORE to 1 to support the 48MHz frequency
    MAP_CS_setExternalClockSourceFrequency(0, FCLOCK);
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    MAP_CS_startHFXT(false);

    // Configure clocks that we need
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_2);

}

void DelfiPQcore::startTaskManager( Task **tasks, const int tasksCount)
{
    // run the initialization code first
    for (int i = 0; i < tasksCount; i++)
    {
        tasks[i]->setUp();
    }

    // run the tasks in a sequence
    while(true)
    {
        for (int i = 0; i < tasksCount; i++)
        {
            tasks[i]->executeTask();
        }
        // low-power mode currently disabled
        // TODO: RS485 frames lost sometimes when low-power mode enables
        //MAP_PCM_gotoLPM0();
    }
}
