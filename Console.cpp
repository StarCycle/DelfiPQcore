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

bool Console::isEnabled()
{
    uint8_t status = MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN2);
    if(status == GPIO_INPUT_PIN_HIGH){
        return true;
    }else{
        return false;
    }
}


void Console::log( const char *text, ... )
{
    if(isEnabled()){
        va_list format_args;
        va_start(format_args, text);

        //initialize string buffer for formating routines.
        //initialize with string to make sure it has a string terminator (for strlen)
        char str_buf[] = "0000000000";

        for ( int ii = 0; ii < strlen(text); ii++ )
        {
            if(text[ii] == '%' && (ii+1 < strlen(text))){ //string formating detected
                ii++;
                switch(text[ii]) {
                    case 's': // string
                        log_insert(va_arg(format_args, char*));
                        break;
                    case 'c':// char
                        UART_transmitData(EUSCI_A0_BASE, va_arg(format_args, char));
                        break;
                    case 'd':// digit
                        log_insert(itoa(str_buf,va_arg(format_args, int),10));
                        break;
                    case 'x':// hexadecimal
                        log_insert(itoa(str_buf,va_arg(format_args, int),16));
                        break;
                    default:
                        break;
                }
            }else{
                UART_transmitData(EUSCI_A0_BASE, text[ii]);
            }
        }
        log();
    }
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

/**** PRIVATE METHODS ****/

// This method is adapted from http://stackoverflow.com/a/10011878/6399671
char* Console::itoa(char* str, uint32_t val, uint8_t base ) {

    int len = strlen(str);
    //Create string based on base value
    for(int i = 1; i <= len; i++) {
        str[len - i] = (uint8_t) ((val % base));
        if (str[len - i] > 9) {
            str[len - i] += 'A' - 10;
        }else{
            str[len - i] += '0';
        }
        val /= base;
    }

    // Filter out all the leading zeroes
    bool reachedStart = false;

    for(int i = 0; i < len; i++) {
        if(str[i] != '0'){
            reachedStart = true;
        }
        if(reachedStart){
            return &str[i];
        }
    }

    //all zero string, return last char
    return &str[len-1];
}

void Console::log_insert( const char *text )
{
    for ( int ii = 0; ii < strlen(text); ii++ )
    {
        UART_transmitData(EUSCI_A0_BASE, text[ii]);
    }
}


