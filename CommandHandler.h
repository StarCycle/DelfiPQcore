/*
 * CommandHandler.h
 *
 *  Created on: 15 Jan 2020
 *      Author: CasperBroekhuizen
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "Service.h"
#include "Task.h"
#include "Service.h"
#include "Console.h"

template <class Frame_Type, class Message_Type>
class CommandHandler: public Task
{
 protected:
     DataBus &bus;
     Service** services;
     int servicesCount;
     Frame_Type rxBuffer, txBuffer;
     Message_Type rxMsg, txMsg;
     void (*onValidCmd)( void );

     virtual void run()
     {
         //Prepare Frame to be send back:
         txBuffer.setDestination(rxBuffer.getSource());
         txBuffer.setSource(bus.getAddress());
         rxMsg.setSize(rxBuffer.getPayloadSize());

         if (rxBuffer.getPayloadSize() > 1)
         {
             bool found = false;

             for (int i = 0; i < servicesCount; i++)
             {
                 if (services[i]->process(rxMsg, txMsg)) // Does any of the Services Handle this command?
                 {
                     // stop the loop if a service is found
                     txBuffer.setPayloadSize(txMsg.getSize());
                     bus.transmit(txBuffer);
                     services[i]->postFunc();
                     found = true;
                     break;
                 }
             }

             if (!found)
             {
                 Console::log("Unknown Service (%d)", (int) rxBuffer.getPayload()[0]);

                 txBuffer.setPayloadSize(2);
                 txBuffer.getPayload()[0] = 0;
                 txBuffer.getPayload()[1] = 0;
                 bus.transmit(txBuffer);
                 return;
             }
             else
             {
                 if (onValidCmd)
                 {
                     onValidCmd();
                 }
                 return;
             }
         }
         else
         {
             // invalid payload size
             // what should we do here?
             Console::log("Invalid Command, size must be > 1");
             txBuffer.setService(0);
             txBuffer.setMessageType(SERVICE_RESPONSE_REPLY);
             txBuffer.getDataPayload()[0] = 1;
             txBuffer.setPayloadSize(1);

             bus.transmit(txBuffer);
             return;
         }
     };

 public:
     CommandHandler(DataBus &interface, Service **servArray, int count) :
         Task(), bus(interface), services(servArray), servicesCount(count)
     {
         onValidCmd = 0;
         rxMsg.setPointer(rxBuffer.getPayload());
         txMsg.setPointer(txBuffer.getPayload());
     };

     void received( DataFrame &newFrame )
     {
         newFrame.copy(rxBuffer);
         if(rxMsg.getMessageType() == SERVICE_RESPONSE_REQUEST){
             notify();
         }
     };

     void onValidCommand(void (*function)( void ))
     {
         onValidCmd = function;
     };
};

#endif /* COMMANDHANDLER_H_ */
