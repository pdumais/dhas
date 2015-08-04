#include "IInsteonMessage.h"

IInsteonMessage::IInsteonMessage()
{
    mBuffer = 0;
    mEchoSize = 0;
}

IInsteonMessage::IInsteonMessage(unsigned char cmd, unsigned char size, unsigned char echoSize)
{
    mSize= size;
    mEchoSize = echoSize;
    mBuffer = new unsigned char[size];
    mBuffer[0] = 0x02;
    mBuffer[1] = cmd;
    mPayload = (unsigned char*)&mBuffer[2];
}

IInsteonMessage::~IInsteonMessage()
{
    if (mBuffer) delete mBuffer;
}

unsigned char IInsteonMessage::getSize()
{
    return mSize;
}

unsigned char IInsteonMessage::getEchoSize()
{
    return mEchoSize;
}

unsigned char* IInsteonMessage::getBuffer()
{
    return mBuffer;
}

unsigned char IInsteonMessage::getMessageID()
{
    return mBuffer[1];
}


std::string IInsteonMessage::toString()
{
    std::string st = "Unknown message " + (unsigned char)mBuffer[1];
    return st;
}
