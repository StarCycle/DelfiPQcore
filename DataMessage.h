/*
 * DataMessage.h
 *
 *  Created on: 20 Jan 2020
 *      Author: Casper
 */

#ifndef DATAMESSAGE_H_
#define DATAMESSAGE_H_

class DataMessage
{
private:
    unsigned char *payloadBuffer;
    unsigned int payloadSize;

public:
    DataMessage(){
        payloadBuffer = 0;
        payloadSize = 0;
    };
    void setPointer(unsigned char *payloadBufferLocation){
        payloadBuffer = payloadBufferLocation;
    };
    void setSize(unsigned int size){
        payloadSize = size;
    };
    unsigned int getSize(){
        return payloadSize;
    };
    unsigned char* getPayload(){
        return payloadBuffer;
    };
};




#endif /* DATAMESSAGE_H_ */
