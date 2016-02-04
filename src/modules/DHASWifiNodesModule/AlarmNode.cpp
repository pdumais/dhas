#include "AlarmNode.h"

REGISTER_NODE_NAME(AlarmNode,"ESP8266 ALARM PGM");

AlarmNode::AlarmNode()
{
    mPgms = 0;
}

AlarmNode::~AlarmNode()
{
}

bool AlarmNode::processData(char* buf, size_t size, Dumais::JSON::JSON& json)
{
    if (size != 2) return false;

    if (buf[0] == 'R' || buf[0]=='S')
    {
        mPgms = buf[1];
        json.addValue("alarmnode","node");
        if (buf[0] == 'S') json.addValue(1,"explicit");
        json.addList("pgms");
        for (int i = 0; i < 8; i++) json["pgms"].addValue(((buf[1]>>i)&1));
        return true;
    }

    return false;
}

void AlarmNode::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack *p;

    p = new RESTCallBack(this,&AlarmNode::status_callback,"Get PGM status");
    pEngine->addCallBack("/dwn/"+mInfo.id+"/status","GET",p);
    mRestCallbacks.push_back(p);
}

void AlarmNode::status_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;

    json.addValue("ok","status");
    json.addList("pgms");
    for (int i = 0; i < 8; i++) json["pgms"].addValue(((mPgms>>i)&1));
}

