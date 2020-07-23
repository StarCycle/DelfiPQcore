/*
 * FRAMBackedFile.h
 *
 *  Created on: 30 Jun 2020
 *      Author: Casper
 */

#ifndef FRAMARRAY_H_
#define FRAMARRAY_H_

#include "MB85RS.h"
#include "Console.h"

template <class varType>
class FRAMArray{

private:
    int arraySize;
    bool autoUpdate;
    unsigned long FRAMAddress;
    MB85RS* FRAM;

public:

    FRAMArray(){
        FRAM = 0;
        FRAMAddress = 0;
        arraySize = 0;
        autoUpdate = false;
    }

    FRAMArray(MB85RS& fram, unsigned long address, int size){
        FRAM = &fram;
        FRAMAddress = address;
        arraySize = size;
        autoUpdate = false;
    }

    void init(MB85RS& fram, unsigned long address,  int size){
        FRAM = &fram;
        FRAMAddress = address;
        arraySize = size;
    }

    varType read(int i){
        varType tmp = 0;
        if(FRAM){
            FRAM->read(FRAMAddress + i*sizeof(varType), (unsigned char*)&tmp, sizeof(varType));
        }
        return tmp;
    }

    void save(int i, varType value){
        if(FRAM){
            FRAM->write(FRAMAddress + i*sizeof(varType), (unsigned char*)&value, sizeof(varType));
        }
    }

    void write(int i, varType value){
        save(i, value);
    }

    int getSize(){
        return arraySize * sizeof(varType);
    }

    varType operator[](int index){
        return this->read(index);
    }
};

#endif /* FRAMBACKEDVAR_H_ */
