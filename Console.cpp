/*
 * Console.cpp
 *
 *  Created on: 28 Mar 2020
 *      Author: stefanosperett
 */

#include "Console.h"



/**** PUBLIC METHODS ****/
void Console::init( unsigned int baudrate )
{
    MAP_UART_disableModule( EUSCI_A0_BASE );   //disable UART operation for configuration settings

    // Selecting P1.2 and P1.3 in UART mode
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
    GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    eUSCI_UART_Config Config;

    //Default Configuration, macro found in uart.h
    Config.selectClockSource    = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    Config.parity               = EUSCI_A_UART_NO_PARITY;
    Config.msborLsbFirst        = EUSCI_A_UART_LSB_FIRST;
    Config.numberofStopBits     = EUSCI_A_UART_ONE_STOP_BIT;
    Config.uartMode             = EUSCI_A_UART_MODE;

    unsigned int n = MAP_CS_getSMCLK() / baudrate;

    if (n > 16)
    {
        Config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION; // Over-sampling
        Config.clockPrescalar = n >> 4;                                      // BRDIV = n / 16
        Config.firstModReg = n - (Config.clockPrescalar << 4);               // UCxBRF = int((n / 16) - int(n / 16)) * 16
    }
    else
    {
        Config.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION; // Low-frequency mode
        Config.clockPrescalar = n;                                            // BRDIV = n
        Config.firstModReg = 0;                                               // UCxBRF not used
    }

    Config.secondModReg = 0;    // UCxBRS = 0

    MAP_UART_initModule( EUSCI_A0_BASE, &Config );

    /* Enable UART module */
    MAP_UART_enableModule( EUSCI_A0_BASE );
}

void Console::log( const char *text )
{
    for ( int ii = 0; ii < strlen(text); ii++ )
    {
        UART_transmitData(EUSCI_A0_BASE, text[ii]);
    }
    log();
}

void Console::log()
{
    MAP_UART_transmitData( EUSCI_A0_BASE, '\r' );
    MAP_UART_transmitData( EUSCI_A0_BASE, '\n' );
}

void Console::flush( void )
{
    // TODO Auto-generated constructor stub

}
