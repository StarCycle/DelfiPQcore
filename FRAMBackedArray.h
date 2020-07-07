/*
 * FRAMBackedFile.h
 *
 *  Created on: 30 Jun 2020
 *      Author: Casper
 */

#ifndef FRAMBACKEDARRAY_H_
#define FRAMBACKEDARRAY_H_

#define FRAMBACKEDARRAY_MAX_SIZE 255

#include "MB85RS.h"
#include "Console.h"

template <class varType>
class FRAMBackedArray{

private:
    varType* RAMcopy; //maximum amount of allowed variables.
    int arraySize;
    unsigned long FRAMAddress;
    MB85RS* FRAM;

public:

    FRAMBackedArray(MB85RS& fram, unsigned long address, varType* RAMcopyPointer,  int size){
        FRAM = &fram;
        FRAMAddress = address;
        arraySize = size;
        RAMcopy = RAMcopyPointer;
    }

    void init(bool updateFromFram){
        if(updateFromFram){
            for(int i = 0; i < arraySize; i++){
                FRAM->read(FRAMAddress + i*sizeof(varType), (unsigned char*)&RAMcopy[i], sizeof(varType));
            }
        }else{
            for(int i = 0; i < arraySize; i++){
                RAMcopy[i] = 0;
                FRAM->write(FRAMAddress + i*sizeof(varType), (unsigned char*)&RAMcopy[i], sizeof(varType));
            }
        }
    }


    varType read(int i){
        return RAMcopy[i];
    }

    int getSize(){
        return arraySize * sizeof(varType);
    }

    void write(int i, varType value){
        RAMcopy[i] = value;
        FRAM->write(FRAMAddress + i*sizeof(varType), (unsigned char*)&RAMcopy[i], sizeof(varType));
    }

    varType operator[](int index){
        return RAMcopy[index];
    }
};

#endif /* FRAMBACKEDVAR_H_ */
