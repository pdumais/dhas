#include "ThermostatService.h"
#include "Logging.h"
#include <list>
#include <sstream>
#include "json/JSON.h"
#include <stdlib.h>
#include <math.h>
#include "HTTPCommunication.h"

#define BUF_SIZE 1024

ThermostatService::ThermostatService()
{
    mServer = "";
}

ThermostatService::~ThermostatService(){
}

void ThermostatService::stop()
{
    Logging::log("Attempting to shutdown Thermostat Service");
}


void ThermostatService::registerCallBacks(RESTEngine* pEngine)
{
    RESTCallBack<ThermostatService> *p;
    p = new RESTCallBack<ThermostatService>(this,&ThermostatService::setTemperature_callback,"Set thermostat setpoint (farenheit) for given  mode");
    p->addParam("t","temperature in fareinheit (integer)");
    p->addParam("mode","cool/heat");
    pEngine->addCallBack("/thermostat/settemperature",p);

    p = new RESTCallBack<ThermostatService>(this,&ThermostatService::setIP_callback,"set IP of thermostat");
    p->addParam("ip","IP address of thermostat");
    pEngine->addCallBack("/thermostat/setip",p);

    p = new RESTCallBack<ThermostatService>(this,&ThermostatService::setMode_callback,"set operating mode");
    p->addParam("mode","cool/heat");
    pEngine->addCallBack("/thermostat/setmode",p);

    p = new RESTCallBack<ThermostatService>(this,&ThermostatService::getStats_callback,"retrieve stats");
    pEngine->addCallBack("/thermostat/getstats",p);

}

void ThermostatService::run()
{
    int temperature = 0;
    mLastPollTime = 0;
    mLastProgramPollTime = 0;
    time_t t=0;
    struct tm *ltm;
    int fanRunning=0;
    int elementRunning=0;
    while (!stopping())
    {
        time(&t);
        if (t >= mLastPollTime)
        {
            mLastPollTime = t+30;
            std::string st = HTTPCommunication::getURL(mServer,"/tstat/");
            if (st!="")
            {
                mStatJson.parse(st);
                if (mStatJson["temp"].toInt()>0) // this will validate valid json and valid temperature.
                {
                    if (mStatJson["temp"].toInt() != temperature)
                    {
                        temperature = mStatJson["temp"].toInt();
                        float tf = round((((temperature)-32)/1.8)*2)/2;
                        Logging::log("Thermostat temperature changed to %f",tf);
                    }

                    if (mStatJson["tstate"].toInt() != elementRunning)
                    {
                        elementRunning = mStatJson["tstate"].toInt();
                        Logging::log("Thermostat element state changed to %i",elementRunning);
                    }
                    if (mStatJson["fstate"].toInt() != fanRunning)
                    {
                        fanRunning = mStatJson["fstate"].toInt();
                        Logging::log("Thermostat fan state changed to %i",fanRunning);
                    }

                    ltm = localtime(&t);
                    unsigned int wday = ltm->tm_wday;
                    if (wday!=0) wday--; else wday=6;
        
                    unsigned int h = mStatJson["time"]["hour"].toUInt();
                    unsigned int m = mStatJson["time"]["minute"].toUInt();
                    unsigned int d = mStatJson["time"]["day"].toUInt();
                    if ((h>=0&&h<=23)&&(m>=0&&m<=59)&&(d>=0&&d<=6))
                    {
                        unsigned int abstime1 = (m*60)+(h*3600)+(d*3600*24);
                        unsigned int abstime2 = (ltm->tm_min*60)+(ltm->tm_hour*3600)+(wday*3600*24);
                        if (abstime1<(abstime2-120) || abstime1>(abstime2+120))
                        {
                            Logging::log("Thermostat time incorrect (day=%i, hour=%i, min=%i). Setting time to day=%i, %i:%i",d,h,m,wday,ltm->tm_hour,ltm->tm_min);
                            Logging::log("fetched: %s",st.c_str());
                            std::stringstream ss;
                            ss<< "{\"time\":{" << "\"hour\":" << ltm->tm_hour << ",\"minute\":" << ltm->tm_min << ",\"day\":" << wday << "}}";
                            HTTPCommunication::postURL(mServer,"/tstat/",ss.str());
                        }
                    }
                    else
                    {
                        //Logging::log("Incorrect time format received by thermostat");
                    }
                }
            }
        }
        usleep(1000000);
    }
}

void ThermostatService::setIP_callback(Dumais::JSON::JSON& json, RESTParameters* params)
{
    mServer = params->getParam("ip");
    json.addValue("ok","status");
}

void ThermostatService::setTemperature_callback(Dumais::JSON::JSON& json, RESTParameters* params)
{
    std::stringstream data;
    std::string t = params->getParam("t");
    std::string mode = params->getParam("mode");

    if (mode == "heat")
    {
        data << "{\"tmode\":1,\"t_heat\":"<<t<<"}";
        HTTPCommunication::postURL(mServer,"/tstat/",data.str());
    }
    else if (mode == "cool")
    {
        data << "{\"tmode\":2,\"t_cool\":"<<t<<"}";
        HTTPCommunication::postURL(mServer,"/tstat/",data.str());
    }

    time_t tm;
    time(&tm);
    mLastPollTime = tm+3;

}

void ThermostatService::getStats_callback(Dumais::JSON::JSON& json, RESTParameters* params)
{
    std::string st = mStatJson.stringify(false);
    json.parse(st);
}

void ThermostatService::setMode_callback(Dumais::JSON::JSON& json, RESTParameters* params)
{
    std::stringstream data;
    std::string mode = params->getParam("mode");

    if (mode == "heat")
    {
        data << "{\"tmode\":1}";
        HTTPCommunication::postURL(mServer,"/tstat/",data.str());
    }
    else if (mode == "cool")
    {
        data << "{\"tmode\":2}";
        HTTPCommunication::postURL(mServer,"/tstat/",data.str());
    }


    time_t t;
    time(&t);
    mLastPollTime = t+3;
}

