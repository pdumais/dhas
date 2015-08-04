#include "StandardOrExtendedMessage.h"
#include <sstream>

StandardOrExtendedMessage::StandardOrExtendedMessage(InsteonID id, unsigned char cmd1, unsigned char cmd2):IInsteonMessage(0x62,8,9)
{
    mPayload[0] = (id >> 16)&0xFF;
    mPayload[1] = (id >> 8)&0xFF;
    mPayload[2] = (id)&0xFF;
    mPayload[3] = 0x0F;
    mPayload[4] = cmd1;
    mPayload[5] = cmd2;
}

StandardOrExtendedMessage::StandardOrExtendedMessage(InsteonID id, unsigned char cmd1, unsigned char cmd2, unsigned char *data,bool i2cs):IInsteonMessage(0x62,22,23)
{
    mPayload[0] = (id >> 16)&0xFF;
    mPayload[1] = (id >> 8)&0xFF;
    mPayload[2] = (id)&0xFF;
    mPayload[3] = 0x1F;
    mPayload[4] = cmd1;
    mPayload[5] = cmd2;
    for (int i=0;i<14;i++) mPayload[i+6]=data[i];

    if (i2cs)
    {
        unsigned int tmp = cmd1 + cmd2;
        for (int i=0;i<13;i++) tmp+=data[i];
        tmp = ((~tmp)+1)&0xFF;
        mPayload[19] = tmp;
    }
}


StandardOrExtendedMessage::~StandardOrExtendedMessage()
{
}

bool StandardOrExtendedMessage::isExtended()
{
    return (mPayload[3] & 0x10);
}

unsigned char StandardOrExtendedMessage::command1()
{
    return mPayload[4];
}

unsigned char StandardOrExtendedMessage::command2()
{
    return mPayload[5];
}

void StandardOrExtendedMessage::copyData(char *buf)
{
    for (int i=0;i<14;i++) buf[i]=mPayload[6+i];
}

InsteonID StandardOrExtendedMessage::getDestination()
{
    return (mPayload[0]<<16)|(mPayload[1]<<8)|mPayload[2];
}

bool StandardOrExtendedMessage::needAck()
{
    return true;
}


std::string StandardOrExtendedMessage::toString()
{
    InsteonID id = (mPayload[0]<<16)|(mPayload[1]<<8)|mPayload[2];
    std::stringstream ss;
    ss << "Direct message, cmd1=0x" << std::hex << static_cast<unsigned int>((unsigned char)mPayload[4]) << ", cmd2=0x" << std::hex << static_cast<unsigned int>((unsigned char)mPayload[5]);
    ss << " To 0x" << std::hex << id;
    return ss.str();
}

