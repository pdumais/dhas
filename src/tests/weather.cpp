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
#include "IDWN.h"

class EventProcessorMock: public IEventProcessor{
public:
    virtual void stop() {};
    virtual void processEvent(Dumais::JSON::JSON &message) {};
    virtual void processSchedule(time_t t) {};
    virtual void scheduleScriptReload() {};
    virtual void checkSun(time_t t) {};
    virtual void appendPeriodicData(Dumais::JSON::JSON& json) {};
};

void testDriverCreation(std::string name)
{
    IDWN* driver = IDWN::createDriverInstance(name);
    if (driver == 0)
    {
        LOG("ERROR: No suitable driver found for wifi node [" << name << "]");
        return;
    }
}

int main(int argc, char** argv) 
{ 
    Dumais::Utils::Logging::logger = new Dumais::Utils::ConsoleLogger();
    Dumais::JSON::JSON j;

    Dumais::JSON::JSON jsonConfig;
    jsonConfig.addObject("DHASWifiNodesModule");

    ModuleProvider serviceProvider(jsonConfig);
    serviceProvider.disableModule("IO");
    serviceProvider.disableModule("phone");
    serviceProvider.disableModule("audio");
    serviceProvider.disableModule("insteon");
    serviceProvider.disableModule("smtp");
    serviceProvider.disableModule("DHASWifiNodes");
    EventProcessorMock epmock;
    RESTInterface *pRESTInterface = new RESTInterface(&serviceProvider,0,0);
    pRESTInterface->init();
    pRESTInterface->processQuery(j,"/weather/setip?ip=192.168.1.20");
    serviceProvider.startModules(&epmock);


    sleep(1);
    pRESTInterface->processQuery(j,"/weather/getstats");
    printf("%s\r\n",j.stringify(true).c_str());
//    sleep(1);

/*    while(1)
    {
        getchar();
        Dumais::JSON::JSON j;
        pRESTInterface->processQuery(j,"/dwn/list");
        LOG(j.stringify(true));
    }*/

    serviceProvider.stopModules();

    return 0; 
}
