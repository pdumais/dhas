#include "WeatherModule.h"
#include "Logging.h"
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
#include "JSON.h"
#include <stdlib.h>
#include <math.h>
#include "HTTPCommunication.h"
#include "regex.h"
#include "ModuleRegistrar.h"


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
    Logging::log("Attempting to shutdown Weather Module");
}


std::string WeatherModule::getTemperature(int id)
{
    if (id<0 || id>3) return "";
    return mTemperatures[id];
}

void WeatherModule::registerCallBacks(RESTEngine* pEngine)
{
    RESTCallBack *p;
    p = new RESTCallBack(this,&WeatherModule::setIP_callback,"set IP of weather controller");
    p->addParam("ip","IP address of weather controller");
    pEngine->addCallBack("/weather/setip","GET",p);

    p = new RESTCallBack(this,&WeatherModule::getStats_callback,"retrieve stats");
    pEngine->addCallBack("/weather/getstats","GET",p);

}

void WeatherModule::run()
{
    mLastPollTime = 0;
    time_t t=0;
    regex_t preg[4];
    regcomp(&preg[0],"<sensor1temp>([-\\.0-9]*)</sensor1temp>",REG_EXTENDED|REG_NEWLINE);
    regcomp(&preg[1],"<sensor2temp>([-\\.0-9]*)</sensor2temp>",REG_EXTENDED|REG_NEWLINE);
    regcomp(&preg[2],"<sensor3temp>([-\\.0-9]*)</sensor3temp>",REG_EXTENDED|REG_NEWLINE);
    regcomp(&preg[3],"<sensor4temp>([-\\.0-9]*)</sensor4temp>",REG_EXTENDED|REG_NEWLINE);
    regmatch_t matches[3];

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
                for (int i=0;i<4;i++)
                {
                    int err = regexec(&preg[i],st.c_str(),2,matches,0);
                    if (err != REG_NOMATCH)
                    {
                        char num[10];
                        int s1 = matches[1].rm_so;
                        int e1 = matches[1].rm_eo;
                        int size = e1-s1;
                        mTemperatures[i] = st.substr(s1,size);
                    }
                }
            }
        }
        usleep(250000);
    }

    for (int i=0;i<4;i++) regfree(&preg[i]);
}


void WeatherModule::setIP_callback(RESTContext context)
{
    RESTParameters* params = context.params;
    Dumais::JSON::JSON& json = context.returnData;
    mServer = params->getParam("ip");
    Logging::log("Weather service server: %s",mServer.c_str());
    json.addValue("ok","status");
}

void WeatherModule::getStats_callback(RESTContext context)
{
    RESTParameters* params = context.params;
    Dumais::JSON::JSON& json = context.returnData;
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
