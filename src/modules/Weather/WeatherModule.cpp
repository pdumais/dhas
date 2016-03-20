#include "WeatherModule.h"
#include "DHASLogging.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <list>
#include <sstream>
#include "json/JSON.h"
#include <stdlib.h>
#include <math.h>
#include "HTTPCommunication.h"
#include "ModuleRegistrar.h"
#include <regex>

#define BUF_SIZE 1024

REGISTER_MODULE(WeatherModule)

WeatherModule::WeatherModule()
{
    mTemperatures[0]="N/A";
    mTemperatures[1]="N/A";
    mTemperatures[2]="N/A";
    mTemperatures[3]="N/A";
    mServer = "";
}

WeatherModule::~WeatherModule(){
}

void WeatherModule::configure(Dumais::JSON::JSON& config)
{
}

void WeatherModule::stop()
{
    LOG("Attempting to shutdown Weather Module");
}


std::string WeatherModule::getTemperature(int id)
{
    if (id<0 || id>3) return "";
    return mTemperatures[id];
}

void WeatherModule::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack *p;
    p = new RESTCallBack(this,&WeatherModule::setIP_callback,"set IP of weather controller");
    p->addParam("ip","IP address of weather controller",false);
    pEngine->addCallBack("/weather/setip","GET",p);

    p = new RESTCallBack(this,&WeatherModule::getStats_callback,"retrieve stats");
    pEngine->addCallBack("/weather/getstats","GET",p);

    p = new RESTCallBack(this,&WeatherModule::getThermostat_callback,"retrieve thermostat status");
    pEngine->addCallBack("/weather/getthermostat","GET",p);

    p = new RESTCallBack(this,&WeatherModule::setpoint_callback,"Configure setpoint");
    p->addParam("temp","value between 10 and 30 with 0.5 increment",false);
    pEngine->addCallBack("/weather/setpoint","GET",p);

    p = new RESTCallBack(this,&WeatherModule::resetfilter_callback,"Reset filter days count");
    pEngine->addCallBack("/weather/resetfilter","GET",p);

    p = new RESTCallBack(this,&WeatherModule::setmode_callback,"Set thermostat running mode");
    p->addParam("mode","heat/cool/off",false);
    pEngine->addCallBack("/weather/setmode","GET",p);

    p = new RESTCallBack(this,&WeatherModule::toggleschedule_callback,"Enable/disable schedule");
    pEngine->addCallBack("/weather/toggleschedule","GET",p);

}

std::string WeatherModule::getXMLValue(const std::string& subject, const std::string& str, int matchindex)
{
    std::smatch match;
    std::regex r(str);
    if (std::regex_search(subject,match,r)) return match[matchindex];
    return "N/A";
}

ThermostatInfo WeatherModule::parseState(const std::string& str)
{
    ThermostatInfo info;
    std::string temp;

    info.temperatures[0] = getXMLValue(str,"<(indoorTemp|sensor1temp)>([-\\.0-9]*)</",2);
    info.temperatures[1] = getXMLValue(str,"<(outdoorTemp|sensor2temp)>([-\\.0-9]*)</",2);
    info.temperatures[2] = getXMLValue(str,"<(humidity|sensor3temp)>([-\\.0-9]*)</",2);
    info.temperatures[3] = getXMLValue(str,"<(sensor4temp)>([-\\.0-9]*)</",2);
    info.setpoint = getXMLValue(str,"<setTemp>([-\\.0-9]*)</",1);
    info.filter = getXMLValue(str,"<filtChng>([0-9]*)</",1);

    temp = getXMLValue(str,"<hold>([0-1]*)</",1);
    if (temp == "11") info.schedule = true; else info.schedule = false;
    temp = getXMLValue(str,"<heatMode>([0-2])</",1);
    if (temp == "2") info.mode = ThermostatInfo::Cool; else if (temp=="1") info.mode=ThermostatInfo::Heat; else info.mode=ThermostatInfo::Off;
    temp = getXMLValue(str,"<heat>([0-1])</",1);
    if (temp == "1") info.heat = true; else info.heat = false;
    temp = getXMLValue(str,"<cool>([0-1])</",1);
    if (temp == "1") info.cool = true; else info.cool = false;
    temp = getXMLValue(str,"<fan>([0-1])</",1);
    if (temp == "1") info.fan = true; else info.fan = false;

    return info;
}

void WeatherModule::run()
{
    mLastPollTime = 0;
    time_t t=0;

    setStarted();
    while (!stopping())
    {
        time(&t);
        if (t >= mLastPollTime)
        {
            mLastPollTime = t+30;
            if (mServer!="")
            {
                std::string st = HTTPCommunication::getURL(mServer,"/state.xml");
                ThermostatInfo info = parseState(st);
                for (int i=0; i<4; i++) mTemperatures[i] = info.temperatures[i];
            }
        }
        usleep(250000);
    }

}


void WeatherModule::setIP_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    mServer = params->getParam("ip");
    LOG("Weather service server: "<<mServer.c_str());
    json.addValue("ok","status");
}

void WeatherModule::convertInfoToJSON(Dumais::JSON::JSON& j, const ThermostatInfo& info)
{
    j.addValue(info.setpoint,"setpoint");
    j.addValue(info.heat,"heat");
    j.addValue(info.cool,"cool");
    j.addValue(info.schedule,"schedule");
    j.addValue(info.filter,"filter");
    j.addValue(info.mode,"mode");
    j.addValue(info.fan,"fan");
}

void WeatherModule::getThermostat_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;

    std::string st = HTTPCommunication::getURL(mServer,"/state.xml");
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::getStats_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    json.addList("temperatures");
    json["temperatures"].addValue(mTemperatures[0]);
    json["temperatures"].addValue(mTemperatures[1]);
    json["temperatures"].addValue(mTemperatures[2]);
    json["temperatures"].addValue(mTemperatures[3]);
}

void WeatherModule::appendPeriodicData(Dumais::JSON::JSON& data)
{
    data.addList("temperatures");
    data["temperatures"].addValue(this->getTemperature(0));
    data["temperatures"].addValue(this->getTemperature(1));
    data["temperatures"].addValue(this->getTemperature(2));
    data["temperatures"].addValue(this->getTemperature(3));
}

void WeatherModule::resetfilter_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string st = HTTPCommunication::getURL(mServer,"/state.xml?rstFilt=1");
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::setpoint_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string temp = params->getParam("temp");
    std::string st = HTTPCommunication::getURL(mServer,"/state.xml?setTemp="+temp);
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::setmode_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string mode = params->getParam("mode");
    std::string modenum = "";
    if (mode == "heat") modenum = "1";
    if (mode == "cool") modenum = "2";
    if (mode == "off") modenum = "0";
    if (modenum == "")
    {
        json.addValue("error","status");
        return;
    }

    std::string st = HTTPCommunication::getURL(mServer,"/state.xml?heatMode="+modenum);
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::toggleschedule_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string st = HTTPCommunication::getURL(mServer,"/state.xml?hold=1");
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}
