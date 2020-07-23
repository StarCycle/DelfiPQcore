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
    bool autoUpdate;
    unsigned long FRAMAddress;
    MB85RS* FRAM;

public:

    FRAMBackedVar(){
        FRAM = 0;
        FRAMAddress = 0;
        autoUpdate = false;
    }

    FRAMBackedVar(MB85RS& fram, unsigned long address){
        FRAM = &fram;
        FRAMAddress = address;
        autoUpdate = false;
    }

    void init(MB85RS& fram, unsigned long address, bool initFromFRAM, bool autoPersistency){
        FRAM = &fram;
        FRAMAddress = address;
        autoUpdate = autoPersistency;

        if(FRAM){
            if(initFromFRAM){
                FRAM->read(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
            }else{
                RAMcopy = 0;
                FRAM->write(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
            }
        }
    }

    void init(bool initFromFRAM, bool autoPersistency){
        autoUpdate = autoPersistency;

        if(FRAM){
            if(initFromFRAM){
                FRAM->read(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
            }else{
                RAMcopy = 0;
                FRAM->write(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
            }
        }
    }

    varType read(){
        return RAMcopy;
    }

    void save(){
        FRAM->write(FRAMAddress, (unsigned char*)&RAMcopy, sizeof(RAMcopy));
    }

    void write(varType value){
        RAMcopy = value;
        if(autoUpdate){
            save();
        }
    }

    int getSize(){
        return sizeof(varType);
    }

    // Overloading of Assignment Operator
    void operator=(varType newValue) {
        write(newValue);
    };
    void operator+=(varType addValue) {
        write(RAMcopy+addValue);
    };
    void operator-=(varType minValue) {
        write(RAMcopy-minValue);
    };


    // Overload typecasting to varType;
    operator varType() const { return RAMcopy; }
    // Overload typecasting to varType*;
    operator varType*() const { return &RAMcopy; }


    // Overload Addition
    varType operator+(varType value) {
         return RAMcopy + value;
    };

    // Overload Subtraction
    varType operator-(varType value) {
         return RAMcopy - value;
    };

};

#endif /* FRAMBACKEDVAR_H_ */
