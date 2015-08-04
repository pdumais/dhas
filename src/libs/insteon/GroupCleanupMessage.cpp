#include "GroupCleanupMessage.h"
#include <sstream>

GroupCleanupMessage::GroupCleanupMessage(InsteonID id, unsigned char cmd1, unsigned char cmd2):IInsteonMessage(0x62,8,9)
{
    mPayload[0] = (id >> 16)&0xFF;
    mPayload[1] = (id >> 8)&0xFF;
    mPayload[2] = (id)&0xFF;
    mPayload[3] = 0x4F;
    mPayload[4] = cmd1;
    mPayload[5] = cmd2;

    // send I2CS message
//    for (int i=0;i<14;i++) mPayload[i+6]=0;
//    unsigned int tmp = cmd1 + cmd2;
//    for (int i=0;i<13;i++) tmp+=mPayload[i+6];
//    tmp = ((~tmp)+1)&0xFF;
//    mPayload[20] = tmp;

}

GroupCleanupMessage::~GroupCleanupMessage()
{
}

unsigned char GroupCleanupMessage::command1()
{
    return mPayload[4];
}

unsigned char GroupCleanupMessage::command2()
{
    return mPayload[5];
}

InsteonID GroupCleanupMessage::getDestination()
{
    return (mPayload[0]<<16)|(mPayload[1]<<8)|mPayload[2];
}

bool GroupCleanupMessage::needAck()
{
    return true;
}


std::string GroupCleanupMessage::toString()
{
    InsteonID id = (mPayload[0]<<16)|(mPayload[1]<<8)|mPayload[2];
    std::stringstream ss;
    ss << "Group Cleanup message, cmd1=0x" << std::hex << static_cast<int>(mPayload[4]) << ", cmd2=0x" << std::hex << static_cast<int>(mPayload[5]);
    ss << " To 0x" << std::hex << id;
    return ss.str();
}

