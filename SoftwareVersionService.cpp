/*
 * SoftwareVersionService.cpp
 *
 *  Created on: 02 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <SoftwareVersionService.h>


extern DSerial serial;

uint8_t HexStringToNibble(char c){
    if ((c) >= 'a'){
        return c - 87;
    }else if ((c) >= 'A'){
        return c - 55;
    }else{
        return c - 48;
    }
}

uint8_t NibblesToByte(uint8_t msb, uint8_t lsb){
    return ((msb << 4) | lsb);
}

SoftwareVersionService::SoftwareVersionService(){
    this->hasVersionNumber = false;
}

SoftwareVersionService::SoftwareVersionService(uint8_t versionString[]){
    this->hasVersionNumber = true;
    this->versionNumber[0] = NibblesToByte(HexStringToNibble(versionString[0]),HexStringToNibble(versionString[1]));
    this->versionNumber[1] = NibblesToByte(HexStringToNibble(versionString[2]),HexStringToNibble(versionString[3]));
    this->versionNumber[2] = NibblesToByte(HexStringToNibble(versionString[4]),HexStringToNibble(versionString[5]));
    this->versionNumber[3] = NibblesToByte(HexStringToNibble(versionString[6]),HexStringToNibble(versionString[7]));
    this->versionNumber[4] = NibblesToByte(HexStringToNibble(versionString[8]),HexStringToNibble(versionString[9]));
    this->versionNumber[5] = NibblesToByte(HexStringToNibble(versionString[10]),HexStringToNibble(versionString[11]));
    this->versionNumber[6] = NibblesToByte(HexStringToNibble(versionString[12]),HexStringToNibble(versionString[13]));
    this->versionNumber[7] = NibblesToByte(HexStringToNibble(versionString[14]),HexStringToNibble(versionString[15]));
}

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   PQ9Bus &interface          Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */
bool SoftwareVersionService::process(PQ9Frame &command, PQ9Sender &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == SOFTWAREVERSION_SERVICE) //Check if this frame is directed to this service
    {
        // prepare response frame
        workingBuffer.setDestination(command.getSource());
        workingBuffer.setSource(interface.getAddress());
        workingBuffer.getPayload()[0] = SOFTWAREVERSION_SERVICE;

        if (command.getPayload()[1] == SOFTWAREVERSION_GETSOFTWAREVERSION)
        {
            workingBuffer.setPayloadSize(2);
            serial.println("SoftwareVersionService: Software Version Request");
            // respond to ping
            workingBuffer.getPayload()[1] = SOFTWAREVERSION_ACCEPT;
            if(this->hasVersionNumber == true){
                workingBuffer.setPayloadSize(10);
                workingBuffer.getPayload()[1] = SOFTWAREVERSION_ACCEPT;
                serial.println("has SW Version!");
                workingBuffer.getPayload()[2] = this->versionNumber[0];
                workingBuffer.getPayload()[3] = this->versionNumber[1];
                workingBuffer.getPayload()[4] = this->versionNumber[2];
                workingBuffer.getPayload()[5] = this->versionNumber[3];
                workingBuffer.getPayload()[6] = this->versionNumber[4];
                workingBuffer.getPayload()[7] = this->versionNumber[5];
                workingBuffer.getPayload()[8] = this->versionNumber[6];
                workingBuffer.getPayload()[9] = this->versionNumber[7];
            }else{
                serial.println("has no SW Version!");
                workingBuffer.setPayloadSize(2);
                workingBuffer.getPayload()[1] = SOFTWAREVERSION_ERROR;
            }
        }
        else
        {
            // unknown request
            workingBuffer.setPayloadSize(2);
            workingBuffer.getPayload()[1] = SOFTWAREVERSION_ERROR;
        }

        // send response
        interface.transmit(workingBuffer);
        // command processed
        return true;
    }
    else
    {
        // this command is related to another service,
        // report the command was not processed
        return false;
    }
}
