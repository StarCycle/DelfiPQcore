/*
 * FRAMVar.h
 *
 *  Created on: 30 Jun 2020
 *      Author: Casper
 */

#ifndef FRAMVAR_H_
#define FRAMVAR_H_

#include "MB85RS.h"
#include "Console.h"
#include "FRAMMap.h"

template <class varType>
class FRAMVar{

private:
    unsigned long FRAMAddress;
    MB85RS* FRAM;

public:

    FRAMVar(){
        FRAM = 0;
        FRAMAddress = 0;
    }

    FRAMVar(MB85RS& fram, unsigned long address){
        FRAM = &fram;
        FRAMAddress = address;
    }

    void init(MB85RS& fram, unsigned long address){
        FRAM = &fram;
        FRAMAddress = address;
    }

    varType read(){
        varType temp;
        if(FRAM){
            FRAM->read(FRAMAddress, (unsigned char*)&temp, sizeof(temp));
        }
        return temp;
    }

    void save(varType value){
        if(FRAM){
            FRAM->write(FRAMAddress, (unsigned char*)&value, sizeof(value));
        }
    }

    void write(varType value){
        save(value);
    }

    int getSize(){
        return sizeof(varType);
    }

    // Overloading of Assignment Operator
    void operator=(varType newValue) {
        write(newValue);
    };
    void operator+=(varType addValue) {
        write(this->read()+addValue);
    };
    void operator-=(varType minValue) {
        write(this->read()-minValue);
    };


    // Overload typecasting to varType;
    operator varType() {
        varType tmp = this->read();
        return tmp;
    }

    // Overload Addition
    varType operator+(varType value) {
         return this->read() + value;
    };

    // Overload Subtraction
    varType operator-(varType value) {
         return this->read() - value;
    };

};

#endif /* FRAMVAR_H_ */
