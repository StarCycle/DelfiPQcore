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

    virtual void setMessageType(unsigned char resp) = 0;
    virtual unsigned char getMessageType() = 0;
    virtual void setService(unsigned char address) = 0;
    virtual unsigned char getService() = 0;
    virtual unsigned char* getDataPayload() = 0;
};




#endif /* DATAMESSAGE_H_ */
