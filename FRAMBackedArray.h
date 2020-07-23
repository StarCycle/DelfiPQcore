/*
 * FRAMBackedFile.h
 *
 *  Created on: 30 Jun 2020
 *      Author: Casper
 */

#ifndef FRAMBACKEDARRAY_H_
#define FRAMBACKEDARRAY_H_

#include "MB85RS.h"
#include "Console.h"

template <class varType>
class FRAMBackedArray{

private:
    varType* RAMcopy; //maximum amount of allowed variables.
    int arraySize;
    bool autoUpdate;
    unsigned long FRAMAddress;
    MB85RS* FRAM;

public:

    FRAMBackedArray(){
        FRAM = 0;
        FRAMAddress = 0;
        RAMcopy = 0;
        arraySize = 0;
        autoUpdate = false;
    }

    FRAMBackedArray(MB85RS& fram, unsigned long address, varType* RAMcopyPointer,  int size){
        FRAM = &fram;
        FRAMAddress = address;
        arraySize = size;
        RAMcopy = RAMcopyPointer;
        autoUpdate = false;
    }

    void init(MB85RS& fram, unsigned long address, varType* RAMcopyPointer,  int size, bool initFromFRAM, bool autoPersistency){
        FRAM = &fram;
        FRAMAddress = address;
        arraySize = size;
        RAMcopy = RAMcopyPointer;
        autoUpdate = autoPersistency;

        if(FRAM){
            if(initFromFRAM){
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
    }

    void init(bool initFromFRAM, bool autoPersistency){
        autoUpdate = autoPersistency;

        if(FRAM){
            if(initFromFRAM){
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
    }


    varType read(int i){
        return RAMcopy[i];
    }

    void save(int i){
        if(FRAM){
            FRAM->write(FRAMAddress + i*sizeof(varType), (unsigned char*)&RAMcopy[i], sizeof(varType));
        }
    }

    void write(int i, varType value){
        RAMcopy[i] = value;
        if(autoUpdate){
            save(i);
        }
    }

    int getSize(){
        return arraySize * sizeof(varType);
    }

    varType operator[](int index){
        return RAMcopy[index];
    }
};

#endif /* FRAMBACKEDVAR_H_ */
