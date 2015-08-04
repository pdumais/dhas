#include "StartAllLinkingMessage.h"
#include <sstream>

StartAllLinkingMessage::StartAllLinkingMessage(unsigned char group, LinkType type):IInsteonMessage(0x64,4,5)
{
    mPayload[0]=type;
    mPayload[1]=group;
}

StartAllLinkingMessage::~StartAllLinkingMessage()
{
}

std::string StartAllLinkingMessage::toString()
{
    std::stringstream ss;
    ss << "Start All Linking message, group=0x" << std::hex << static_cast<int>(mPayload[0]) << ", type=0x" << std::hex << static_cast<int>(mPayload[1]);
    return ss.str();

}

InsteonID StartAllLinkingMessage::getDestination()
{
    return 0;
}

bool StartAllLinkingMessage::needAck()
{
    return true;
}

