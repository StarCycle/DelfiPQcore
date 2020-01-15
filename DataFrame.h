/*
 * DataBus.h
 *
 *  Created on: 14 Jan 2020
 *      Author: CasperBroekhuizen
 */

#ifndef _DATAFRAME_H_
#define _DATAFRAME_H_

class DataFrame
{
public:
    virtual ~DataFrame(){}
    virtual unsigned char getDestination() = 0;
    virtual void setDestination(unsigned char destination) = 0;
    virtual unsigned char getSource() = 0;
    virtual void setSource(unsigned char source) = 0;
    virtual unsigned char getPayloadSize() = 0;
    virtual void setPayloadSize(unsigned char size) = 0;
    virtual unsigned char *getPayload() = 0;
    virtual void copy(DataFrame &destination) = 0;

    virtual void PrepareTransmit() = 0;
    virtual unsigned int getFrameSize() = 0;
    virtual unsigned char *getFrame() = 0;
};

#endif  /* _DATABUS_H_ */
