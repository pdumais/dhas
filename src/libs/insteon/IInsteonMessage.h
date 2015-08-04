#pragma once
#include <string>
#include "InsteonDefs.h"

class IInsteonMessage
{
private:
    unsigned char *mBuffer;
    unsigned char mSize;
    unsigned char mEchoSize;
       
protected:
    unsigned char *mPayload; 
public:
    IInsteonMessage();
    IInsteonMessage(unsigned char cmd, unsigned char size, unsigned char echoSize);
    virtual ~IInsteonMessage();
    unsigned char getSize();
    unsigned char getEchoSize();
    unsigned char* getBuffer();
    
    unsigned char getMessageID();
    virtual std::string toString();
    virtual InsteonID getDestination()=0;
    virtual bool needAck()=0;

};
