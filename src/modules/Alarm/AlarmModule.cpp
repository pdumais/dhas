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
    this->mpSerialPort = new IPSerialPort(config["ip"].str(),23);
}

void AlarmModule::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
}


void AlarmModule::run()
{
    AlarmData data;

    setStarted();
    while (!stopping())
    {
        this->mpSerialPort->Reconnect();
        memset(&data,0,sizeof(AlarmData));
        int r = this->mpSerialPort->Read((unsigned char*)&data, sizeof(AlarmData));
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
                else if (data.subgroup == 12)
                {
                    //j.addValue("Armed","action");
                }
                else if (data.subgroup == 11)
                {
                    //j.addValue("disarmed","action");
                }
        
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
            
            std::string label;
            size_t n = 15;
            for (int i=15;i>=0;i--)
            {
                if (data.label[i]==0 || data.label[i]==' ')
                {
                    n--;
                }
                else
                {
                    break;
                }
            }

            label.assign(data.label,n+1);
            j.addValue(label,"label");
            mpEventProcessor->processEvent(j);
        }
        sleep(1);
    }
}

void AlarmModule::stop()
{
}


