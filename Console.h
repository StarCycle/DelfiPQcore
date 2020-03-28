/*
 * Console.h
 *
 *  Created on: 28 Mar 2020
 *      Author: stefanosperett
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <driverlib.h>
extern "C" {
#include<string.h>
}

class Console
{
private:

    // Private constructor to prevent instancing.
    Console();

public:

    static void init( unsigned int baudrate );
    static void log( const char *text );
    static void log( void );
    static void flush( void );

};

#endif /* CONSOLE_H_ */
