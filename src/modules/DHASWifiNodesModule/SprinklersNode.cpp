#include "DHASLogging.h"
#include "SprinklersNode.h"

#define TIMETMAX std::numeric_limits<time_t>::max()

REGISTER_NODE_NAME(SprinklersNode,"ESP8266 IRRIGATION CONTROLLER");

SprinklersNode::SprinklersNode()
{
    mpSequence = new Dumais::Utils::MPSCRingBuffer<SequenceItem>(100);
    mNextSequenceTime = TIMETMAX;
}

SprinklersNode::~SprinklersNode()
{
    delete mpSequence;
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

    p = new RESTCallBack(this,&SprinklersNode::sequence_callback,"Activate a sequence");
    p->addParam("seq","example: a3,b6,d3,c4 will turn on valve A for 3s, then B for 6s, D for 3s and finally c for 4s",false);
    pEngine->addCallBack("/dwn/"+mInfo.id+"/sequence","GET",p);
    mRestCallbacks.push_back(p);
}


std::vector<std::pair<char,int>> decodeSequence(const std::string& seq)
{
    std::vector<std::pair<char,int>> list;
    std::regex r("(([a-z])([0-9]*))");    
 
    try
    {
        auto matches = std::sregex_iterator(seq.begin(),seq.end(), r);
        for (auto it = matches; it != std::sregex_iterator(); it++)
        {
            list.push_back(std::pair<char,int>((*it).str(2)[0],std::stoul((*it).str(3))));
    
        }
    }
    catch (std::exception)
    {
        return std::vector<std::pair<char,int>>();
    }

    return list;
}


void SprinklersNode::sequence_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;

    clearSequence();
    auto list = decodeSequence(params->getParam("seq"));
    mCurrentSequence = params->getParam("seq");;
    for (auto& it : list)
    {
        addInSequence(it.first-'a',it.second);
    }
    json.addValue("ok","status");
    json.addValue((int)list.size(),"items");
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
    clearSequence();

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

void SprinklersNode::run(time_t t)
{
    if (t < mNextSequenceTime) return;

    SequenceItem si;
    if (!mpSequence->get(si))
    {
        LOG("Sequence complete, stopping all valves");
        // If the last item in queue was terminated, stop all valves
        char buf[2];
        buf[0] = 'a'-1;  // will evaluate to 0 on node
        buf[1] = '0';
        this->mInfo.sendQueue->sendToNode(this->mInfo.ip,buf,2);

        mNextSequenceTime = TIMETMAX;
        mCurrentSequence = "";
        return;
    }

    mNextSequenceTime = time(0) + si.first;
    LOG("Activate valve " << (int)si.second << " as part of sequence. Next: " << (unsigned int)mNextSequenceTime);
    char buf[2];
    buf[0] = 'a'+si.second;
    buf[1] = '1';
    this->mInfo.sendQueue->sendToNode(this->mInfo.ip,buf,2);
}
    
void SprinklersNode::clearSequence()
{
    mCurrentSequence = "";
    SequenceItem si;
    mNextSequenceTime = TIMETMAX;
    while (mpSequence->get(si));
}

void SprinklersNode::addInSequence(unsigned char valve, unsigned int duration)
{
    LOG("Adding valve " << (int)valve << " in sequence with duration " << duration);
    mpSequence->put(SequenceItem(duration,valve));
    if (mNextSequenceTime == TIMETMAX) mNextSequenceTime = time(0);
}

void SprinklersNode::addInfo(Dumais::JSON::JSON& obj)
{
    obj.addValue(mValveStatus,"valves");
    obj.addValue(mCurrentSequence,"sequence");
}
