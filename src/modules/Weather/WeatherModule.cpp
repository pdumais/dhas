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


std::string WeatherModule::getTemperature(int devid, int id)
{
    if (devid >= mDevices.size()) return "";
    Device dev = mDevices[devid];
    if (id<0 || id>3) return "";
    return dev.mTemperatures[id];
}

void WeatherModule::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack *p;
    p = new RESTCallBack(this,&WeatherModule::setIP_callback,"set IP of weather controller");
    p->addParam("ip","IP address of weather controller",false);
    pEngine->addCallBack("/weather/setip","GET",p);

    p = new RESTCallBack(this,&WeatherModule::getStats_callback,"retrieve stats");
    p->addParam("id","device index",false);
    pEngine->addCallBack("/weather/getstats","GET",p);

    p = new RESTCallBack(this,&WeatherModule::getThermostat_callback,"retrieve thermostat status");
    p->addParam("id","device index",false);
    pEngine->addCallBack("/weather/getthermostat","GET",p);

    p = new RESTCallBack(this,&WeatherModule::setpoint_callback,"Configure setpoint");
    p->addParam("id","device index",false);
    p->addParam("temp","value between 10 and 30 with 0.5 increment",false);
    pEngine->addCallBack("/weather/setpoint","GET",p);

    p = new RESTCallBack(this,&WeatherModule::resetfilter_callback,"Reset filter days count");
    p->addParam("id","device index",false);
    pEngine->addCallBack("/weather/resetfilter","GET",p);

    p = new RESTCallBack(this,&WeatherModule::setmode_callback,"Set thermostat running mode");
    p->addParam("id","device index",false);
    p->addParam("mode","heat/cool/off",false);
    pEngine->addCallBack("/weather/setmode","GET",p);

    p = new RESTCallBack(this,&WeatherModule::toggleschedule_callback,"Enable/disable schedule");
    p->addParam("id","device index",false);
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
    if (temp == "2") info.mode = Cool; else if (temp=="1") info.mode=Heat; else info.mode=Off;
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
            int i = 0;
            for (auto& dev : mDevices)
            {
                if (dev.mServer!="")
                {
                    std::string st = HTTPCommunication::getURL(dev.mServer,"/state.xml");
                    ThermostatInfo info = parseState(st);
                    for (int i=0; i<4; i++) dev.mTemperatures[i] = info.temperatures[i];
                    HeatMode temp = dev.mRunStatus;
                    if (info.cool) dev.mRunStatus = Cool; else if (info.heat) dev.mRunStatus = Heat; else dev.mRunStatus = Off;
                    if (dev.mRunStatus != temp)
                    {
                        Dumais::JSON::JSON json;
                        json.addValue(i,"devid");
                        json.addValue("thermostat","event");
                        json.addValue("modechange","type");
                        json.addValue(dev.mRunStatus,"status");
                        mpEventProcessor->processEvent(json);
                    }
                }
                i++;
            }
        }
        usleep(250000);
    }

}


void WeatherModule::setIP_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    Device dev; 
    dev.mServer = params->getParam("ip");
    dev.mTemperatures[0]="N/A";
    dev.mTemperatures[1]="N/A";
    dev.mTemperatures[2]="N/A";
    dev.mTemperatures[3]="N/A";
    dev.mRunStatus = Off;

    //TODO: race condition with run() thread
    mDevices.push_back(dev);
    LOG("Weather service server: "<< dev.mServer.c_str());
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
    int devid = 0;
    try { devid = std::stoi(params->getParam("id"));} catch(std::exception& e){devid=0;}
    if (devid >= mDevices.size()) return;

    Device dev = mDevices[devid];
    
    std::string st = HTTPCommunication::getURL(dev.mServer,"/state.xml");
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::getStats_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    int devid = 0;
    try { devid = std::stoi(params->getParam("id"));} catch(std::exception& e){devid=0;}
    if (devid >= mDevices.size()) return;
    Device dev = mDevices[devid];

    json.addList("temperatures");
    json["temperatures"].addValue(dev.mTemperatures[0]);
    json["temperatures"].addValue(dev.mTemperatures[1]);
    json["temperatures"].addValue(dev.mTemperatures[2]);
    json["temperatures"].addValue(dev.mTemperatures[3]);
}

void WeatherModule::appendPeriodicData(Dumais::JSON::JSON& data)
{
    data.addValue(mDevices[0].mRunStatus,"thermostatrunstatus");
    data.addList("temperatures");
    int devid = 0;
    for (auto& dev : mDevices)
    {
        for (int i = 0; i < 4; i++)
        {
            data["temperatures"].addValue(this->getTemperature(devid,i));
        }
        devid++;
    }
}

void WeatherModule::resetfilter_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    int devid = 0;
    try { devid = std::stoi(params->getParam("id"));} catch(std::exception& e){devid=0;}
    if (devid >= mDevices.size()) return;
    Device dev = mDevices[devid];

    std::string st = HTTPCommunication::getURL(dev.mServer,"/state.xml?rstFilt=1");
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::setpoint_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    int devid = 0;
    try { devid = std::stoi(params->getParam("id"));} catch(std::exception& e){devid=0;}
    if (devid >= mDevices.size()) return;
    Device dev = mDevices[devid];

    std::string temp = params->getParam("temp");
    std::string st = HTTPCommunication::getURL(dev.mServer,"/state.xml?setTemp="+temp);
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::setmode_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    int devid = 0;
    try { devid = std::stoi(params->getParam("id"));} catch(std::exception& e){devid=0;}
    if (devid >= mDevices.size()) return;
    Device dev = mDevices[devid];
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

    std::string st = HTTPCommunication::getURL(dev.mServer,"/state.xml?heatMode="+modenum);
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}

void WeatherModule::toggleschedule_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    int devid = 0;
    try { devid = std::stoi(params->getParam("id"));} catch(std::exception& e){devid=0;}
    if (devid >= mDevices.size()) return;
    Device dev = mDevices[devid];
    std::string st = HTTPCommunication::getURL(dev.mServer,"/state.xml?hold=1");
    ThermostatInfo info = parseState(st);
    convertInfoToJSON(json,info);
}
