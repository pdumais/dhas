#include "DHASLogging.h"
#include "AlarmModule.h"
#include "IPSerialPort.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ModuleRegistrar.h"


struct AlarmData
{
    char pad1[7];
    unsigned char group;
    unsigned char subgroup;
    unsigned char partition;
    char pad2[4];
    unsigned char labelType;
    char label[16];
    char pad3[6];
};

REGISTER_MODULE(AlarmModule)

AlarmModule::AlarmModule()
{
    this->mpSerialPort = 0;
}

AlarmModule::~AlarmModule()
{
    if (this->mpSerialPort)
    {
        delete this->mpSerialPort;
    }
}

void AlarmModule::configure(Dumais::JSON::JSON& config)
{
    if (this->mpSerialPort)
    {
        delete this->mpSerialPort;
    }

    this->mpSerialPort = new IPSerialPort(config["ip"].str(),23);
}

void AlarmModule::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
}


void AlarmModule::run()
{
    AlarmData data;

    bool connected = false;
    setStarted();
    while (!stopping())
    {
        connected = this->mpSerialPort->Reconnect();


        memset(&data,0,sizeof(AlarmData));
        int r = 0;
        if (connected)
        {
            r = this->mpSerialPort->Read((unsigned char*)&data, sizeof(AlarmData));
        }

        if (r == sizeof(AlarmData))
        {
            Dumais::JSON::JSON j;
            j.addValue("alarm","event");
            
            if (data.group == 0)
            {
                j.addValue("zoneon","action");
            }
            else if (data.group == 1)
            {
                j.addValue("zoneoff","action");
            }
            else if (data.group == 2)
            {
                if (data.subgroup == 14)
                {
                    j.addValue("Exit delay","action");
                }
                else if (data.subgroup == 13)
                {
                    j.addValue("Entry delay","action");
                }
                else 
                {
                    j.addValue("Unknown-A","action");
                    j.addValue((int)data.subgroup,"subgroup");
                    j.addValue((int)data.group,"group");
                }
                /*else if (data.subgroup == 12)
                {
                    j.addValue("Armed","action");
                }
                else if (data.subgroup == 11)
                {
                    j.addValue("disarmed","action");
                }*/
        
            }
            else if (data.group == 29)
            {
                j.addValue("Armed","action");
                j.addValue((int)data.subgroup,"usernumber");
            }
            else if (data.group == 31)
            {
                j.addValue("Disarmed","action");
                j.addValue((int)data.subgroup,"usernumber");
            }
            else 
            {
                j.addValue("Unknown-B","action");
                j.addValue((int)data.subgroup,"subgroup");
                j.addValue((int)data.group,"group");
            }
            
            std::string label;
            for (int i=0;i<=15;i++) if (data.label[i]==0) data.label[i]=' ';
            label.assign(data.label,15);
            label.erase(label.begin(),std::find_if(label.begin(),label.end(),[](int c){return !std::isspace(c);}));
            label.erase(std::find_if(label.rbegin(),label.rend(),[](int c){return !std::isspace(c);}).base(),label.end());

            j.addValue(label,"label");
            mpEventProcessor->processEvent(j);
        }
        sleep(1);
    }
}

void AlarmModule::stop()
{
}


