#include "DHASLogging.h"
#include "IOBoard.h"

REGISTER_NODE_NAME(IOBoard,"EDISON IO CONTROLLER");

IOBoard::IOBoard()
{
    mPgms = 0;
    mPgmCount = 0;
}

IOBoard::~IOBoard()
{
}

bool IOBoard::processData(char* buf, size_t size, Dumais::JSON::JSON& json)
{
    std::string st(buf,size);
    Dumais::JSON::JSON data;
    data.parse(st);
    LOG("IOBoard received " << st);
    if (data["type"].str() == "initial")
    {
        json.addValue("ioboardnode","node");
        json.addList("pgms");
        mPgmCount = data["pins"].size();
        mPgms = 0;
        for (int i = 0; i < mPgmCount; i++)
        {
            uint8_t tmp = 0;
            if (data["pins"][i].toBool()) tmp = 1; 
            mPgms |= (tmp << i);
            json["pgms"].addValue(tmp);
        }

        return true;
    }
    else if (data["type"].str() == "alarm")
    {
        json.addValue("ioboardnode","node");
        json.addValue("alarmpanel","subtype");

        int group = data["group"].toInt();
        int subGroup = data["subgroup"].toInt();
        std::string label = data["label"].str();
        json.addValue(label,"zonelabel");

        if (group == 0)         // Zone OK
        {
            json.addValue("zoneoff","action");
            return true;

        } 
        else if (group == 1)    // Zone open
        {
            json.addValue("zoneon","action");
            return true;
        }
        else if (group == 2)    // Partition status
        {
            if (subGroup == 14)
            {
                json.addValue("Exit delay","action");
                return true;
            }
            else if (subGroup == 12)
            {
//                json.addValue("Armed","action");
//                return true;
            }
            else if (subGroup == 13)
            {
                json.addValue("Entry delay","action");
                return true;
            }
            else if (subGroup == 11)
            {
//                json.addValue("Disarmed","action");
//                return true;
            }
        }
        else if (group == 29)
        {
            json.addValue("Armed","action");
            json.addValue(subGroup,"usernumber");    
            return true;
        }
        else if (group == 31)
        {
            json.addValue("Disarmed","action");
            json.addValue(subGroup,"usernumber");    
            return true;
        }
        return false;

    }
    else if (data["type"].str() == "changed")
    {
        json.addValue("ioboardnode","node");
        json.addValue("changed","subtype");
        json.addValue(data["pin"].toInt(),"pin");
        json.addList("pgms");
        mPgmCount = data["pins"].size();
        mPgms = 0;
        for (int i = 0; i < mPgmCount; i++)
        {
            uint8_t tmp = 0;
            if (data["pins"][i].toBool()) tmp = 1;
            mPgms |= (tmp << i);
            json["pgms"].addValue(tmp);
        }

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

    p = new RESTCallBack(this,&IOBoard::setoutput_callback,"set output");
    pEngine->addCallBack("/dwn/"+mInfo.id+"/set","GET",p);
    p->addParam("out","0-4",false);
    p->addParam("level","0 or 1",false);
    mRestCallbacks.push_back(p);
}

void IOBoard::status_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;

    json.addValue("ok","status");
    json.addList("pgms");
    for (int i = 0; i < mPgmCount; i++) json["pgms"].addValue(((mPgms>>i)&1));
}

void IOBoard::setoutput_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string out = params->getParam("out");
    std::string level = params->getParam("level");

    Dumais::JSON::JSON j;
    j.addValue("set","event");
    j.addValue(out,"out");
    j.addValue(level,"level");

    std::string st = j.stringify(false);
    this->mInfo.sendQueue->sendToNode(this->mInfo.ip,st.c_str(),st.size());
}
