#include "DHASLogging.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include "WebInterface.h"
#include "ModuleProvider.h"
#include "RESTInterface.h"
#include "IEventProcessor.h"
#include "WebNotificationEngine.h"
#include <sched.h>
#include "config.h"
#include "WeatherHelper.h"
#include <execinfo.h>
#include <signal.h>
#include "ModuleRegistrar.h"

class EventProcessorMock: public IEventProcessor{
public:
    virtual void stop() {};
    virtual void processEvent(Dumais::JSON::JSON &message) {};
    virtual void processSchedule(time_t t) {};
    virtual void scheduleScriptReload() {};
    virtual void checkSun(time_t t) {};
    virtual void appendPeriodicData(Dumais::JSON::JSON& json) {};
};


int main(int argc, char** argv) 
{ 
    Dumais::Utils::Logging::logger = new Dumais::Utils::ConsoleLogger();
    LOG("Starting\r\n");

    std::string sounddev = "default";
    if (argc > 1)
    {
        sounddev = argv[1];
    }

    Dumais::JSON::JSON jsonConfig;
    jsonConfig.addObject("smtp");
    jsonConfig.addObject("web");
    jsonConfig.addObject("phone");
    jsonConfig.addObject("audio");
    jsonConfig.addObject("insteon");
    jsonConfig["phone"].addValue(SOUNDS_FOLDER,"soundsfolder");
    jsonConfig["phone"].addValue(RTP_LOW_PORT+1000,"rtplow");
    jsonConfig["phone"].addValue(RTP_HIGH_PORT+1000,"rtphigh");
    jsonConfig["phone"].addValue(LOCAL_IP,"localip");
    jsonConfig["phone"].addValue(5588,"sipport");
    jsonConfig["phone"].addValue(RONA_TIMEOUT,"ronatimeout");
    jsonConfig["audio"].addValue(sounddev,"sounddevice");
    jsonConfig["audio"].addValue(SOUNDS_FOLDER,"soundsfolder");


    ModuleProvider serviceProvider(jsonConfig);
    serviceProvider.disableModule("DHASWifiNodes");
    serviceProvider.disableModule("IO");
    serviceProvider.disableModule("Weather");
    serviceProvider.disableModule("insteon");
    serviceProvider.disableModule("smtp");
    EventProcessorMock epmock;
    RESTInterface *pRESTInterface = new RESTInterface(&serviceProvider,0,0);
LOG("meow");
    pRESTInterface->init();
LOG("woof");
    serviceProvider.startModules(&epmock);

    Dumais::JSON::JSON j;
//    sleep(1);
    LOG("meow");
    pRESTInterface->processQuery(j,"/audio/play?sound=pop");
//    pRESTInterface->processQuery(j,"/phone/register?user=dhastests&pin=dhaspass&proxy=192.168.1.3:5070");
  //  sleep(1);
    //pRESTInterface->processQuery(j,"/phone/call?ext=711&play=mange,$4,mange");
//    pRESTInterface->processQuery(j,"/phone/call?ext=711&play=$1,mange");

    //while(1);

    serviceProvider.stopModules();

    return 0; 
}
