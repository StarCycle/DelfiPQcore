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
    unsigned char *MessageBuffer;
    unsigned int MessageSize;

public:
    DataMessage(){
        MessageBuffer = 0;
        MessageSize = 0;
    };
    void setPointer(unsigned char *payloadBufferLocation){
        MessageBuffer = payloadBufferLocation;
    };
    void setSize(unsigned int size){
        MessageSize = size;
    };
    unsigned int getSize(){
        return MessageSize;
    };
    unsigned char* getBuffer(){
        return MessageBuffer;
    };

    virtual void setMessageType(unsigned char resp) = 0;
    virtual unsigned char getMessageType() = 0;
    virtual void setService(unsigned char address) = 0;
    virtual unsigned char getService() = 0;
    virtual unsigned char* getDataPayload() = 0;

    virtual void setPayloadSize(unsigned int size) = 0;
    virtual unsigned int getPayloadSize() = 0;

    //This should become deprecated ASAP, as the naming became confusing,
    //Its available for now for older services.
    unsigned char* getPayload(){
        return getBuffer();
    };
};




#endif /* DATAMESSAGE_H_ */
