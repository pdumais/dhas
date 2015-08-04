#include "AllLinkDatabaseMessage.h"
#include <sstream>

AllLinkDatabaseMessage::AllLinkDatabaseMessage(bool first):IInsteonMessage(first?0x69:0x6A,2,3)
{
}

AllLinkDatabaseMessage::~AllLinkDatabaseMessage(){
}

std::string AllLinkDatabaseMessage::toString()
{
    std::stringstream ss;
    ss << "All-link database message 0x" <<  std::hex << static_cast<int>(getMessageID());
    return ss.str();
}

InsteonID AllLinkDatabaseMessage::getDestination()
{
    return 0;
}

bool AllLinkDatabaseMessage::needAck()
{
    return true;
}

