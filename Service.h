/*
 * Service.h
 *
 *  Created on: 25 Jul 2019
 *      Author: stefanosperett
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include "DataBus.h"
#include "DataFrame.h"
#include "DataMessage.h"

#define SERVICE_RESPONSE_ERROR      0
#define SERVICE_RESPONSE_REQUEST    1
#define SERVICE_RESPONSE_REPLY      2

class Service
{
 protected:
    void (*PostFunction)( void ) = 0;
 public:


    virtual ~Service( ) {};
    virtual bool process(DataMessage &command, DataMessage &workingBbuffer) = 0;
    void setPostFunc( void (*userFunc)(void)){
        PostFunction = userFunc;
    };
    void postFunc(){
        if(PostFunction){
            PostFunction();
            PostFunction = 0;
        }
    };
};

#endif /* SERVICE_H_ */
