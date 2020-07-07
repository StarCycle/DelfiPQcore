/*
 * FRAMBackedFile.h
 *
 *  Created on: 30 Jun 2020
 *      Author: Casper
 */

#ifndef FRAMBACKEDVAR_H_
#define FRAMBACKEDVAR_H_

#include "MB85RS.h"
#include "Console.h"
#include "FRAMMap.h"

template <class varType>
class FRAMBackedVar{

private:
    varType RAMcopy;
    unsigned long FRAMAddress;
    MB85RS* FRAM;

public:
    FRAMBackedVar(MB85RS& fram, unsigned long address){
        FRAM = &fram;
        FRAMAddress = address;
    }

    void init(bool updateFromFram){
        if(updateFromFram){
            FRAM->read(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
        }else{
            RAMcopy = 0;
            FRAM->write(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
        }
    }


    varType read(){
        return RAMcopy;
    }

    void write(varType value){
        RAMcopy = value;
        FRAM->write(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
    }

    int getSize(){
        return sizeof(varType);
    }

    // Overloading of Assignment Operator
    varType operator=(varType newValue) {
        write(newValue);
    }

    // Overload typecasting to varType;
    operator varType() const { return RAMcopy; }

    // Overload Addition
    varType operator+(varType value) {
         return RAMcopy + value;
    }

    // Overload Subtraction
    varType operator-(varType value) {
         return RAMcopy - value;
    }

};

#endif /* FRAMBACKEDVAR_H_ */
