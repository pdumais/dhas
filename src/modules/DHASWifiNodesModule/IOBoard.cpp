#include "IOBoard.h"

REGISTER_NODE_NAME(IOBoard,"ESP8266 IO BOARD");

IOBoard::IOBoard()
{
    mPgms = 0;
}

IOBoard::~IOBoard()
{
}

bool IOBoard::processData(char* buf, size_t size, Dumais::JSON::JSON& json)
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

void IOBoard::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack *p;

    p = new RESTCallBack(this,&IOBoard::status_callback,"Get PGM status");
    pEngine->addCallBack("/dwn/"+mInfo.id+"/status","GET",p);
    mRestCallbacks.push_back(p);
}

void IOBoard::status_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;

    json.addValue("ok","status");
    json.addList("pgms");
    for (int i = 0; i < 8; i++) json["pgms"].addValue(((mPgms>>i)&1));
}

