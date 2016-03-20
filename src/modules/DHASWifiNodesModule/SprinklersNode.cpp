#include "SprinklersNode.h"

REGISTER_NODE_NAME(SprinklersNode,"ESP8266 IRRIGATION CONTROLLER");

SprinklersNode::SprinklersNode()
{
}

SprinklersNode::~SprinklersNode()
{
}

bool SprinklersNode::processData(char* buf, size_t size, Dumais::JSON::JSON& json)
{
    if (size != 2) return false;

    if (buf[0] == 'R' || buf[0]=='S')
    {   
        json.addValue("sprinklernode","node");
    
        uint8_t valve = 0;
        for (int i = 0; i <8; i++)
        {
            if ((1<<i)&buf[1]) valve = i+1;
        }
        mValveStatus = valve;

        if (buf[0] == 'R')
        {
            json.addValue(valve,"valvechanged");
        }
        else
        {
            json.addValue(valve,"valvestatus");
        }
        
        return true;
    }

    return false;
}
    
void SprinklersNode::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack *p;

    p = new RESTCallBack(this,&SprinklersNode::valveOn_callback,"Activate a sprinkler");
    p->addParam("valve","a,b,c or d",false);
    pEngine->addCallBack("/dwn/"+mInfo.id+"/on","GET",p);
    mRestCallbacks.push_back(p);

    p = new RESTCallBack(this,&SprinklersNode::valveOff_callback,"Turn off all sprinklers");
    pEngine->addCallBack("/dwn/"+mInfo.id+"/off","GET",p);
    mRestCallbacks.push_back(p);

    p = new RESTCallBack(this,&SprinklersNode::status_callback,"get sprinklers status");
    pEngine->addCallBack("/dwn/"+mInfo.id+"/status","GET",p);
    mRestCallbacks.push_back(p);
}


void SprinklersNode::valveOn_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string valve = params->getParam("valve");
    if (valve[0]<'a' || valve[0]>'d')
    {
        json.addValue("No such valve","status");
        return;
    }

    char buf[2];
    buf[0] = valve[0];
    buf[1] = '1';
    this->mInfo.sendQueue->sendToNode(this->mInfo.ip,buf,2);

    json.addValue("ok","status");
}

void SprinklersNode::valveOff_callback(RESTContext* context)
{
    char buf[2];
    buf[0] = 'a'-1;  // will evaluate to 0 on node
    buf[1] = '0';
    this->mInfo.sendQueue->sendToNode(this->mInfo.ip,buf,2);
}

void SprinklersNode::status_callback(RESTContext* context)
{
    Dumais::JSON::JSON& json = context->returnData;
    json.addValue("ok","status");
    json.addValue(mValveStatus,"valves");
}
