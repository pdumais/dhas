#include "IMConfigurationMessage.h"
#include <sstream>

IMConfigurationMessage::IMConfigurationMessage(unsigned char config):IInsteonMessage(0x6B,3,4)
{
    mPayload[0] = config;
}

IMConfigurationMessage::~IMConfigurationMessage(){
}

std::string IMConfigurationMessage::toString()
{
    std::stringstream ss;
    ss << "IM configuration message. param=0x" <<std::hex << static_cast<int>(mPayload[0]);
    return ss.str();
}

InsteonID IMConfigurationMessage::getDestination()
{
    return 0;
}

bool IMConfigurationMessage::needAck()
{
    return false;
}

