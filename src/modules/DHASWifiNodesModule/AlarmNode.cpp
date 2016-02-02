#include "AlarmNode.h"

REGISTER_NODE_NAME(AlarmNode,"ESP8266 ALARM PGM");

AlarmNode::AlarmNode()
{
}

AlarmNode::~AlarmNode()
{
}

bool AlarmNode::processData(char* buf, size_t size, Dumais::JSON::JSON& json)
{
    if (size != 2) return false;

    if (buf[0] == 'R' || buf[0]=='S')
    {
        json.addValue("alarmnode","node");
        json.addList("pgms");
        for (int i = 0; i < 8; i++) json["pgms"].addValue(((buf[1]>>i)&1));

        return true;
    }

    return false;
}

void AlarmNode::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
}


