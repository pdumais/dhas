#include "Logging.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include "WebInterface.h"
#include "ModuleProvider.h"
#include "RESTInterface.h"
#include "EventProcessor.h"
#include "WebNotificationEngine.h"
#include <sched.h>
#include "config.h"
#include "WeatherHelper.h"
#include <execinfo.h>
#include <signal.h>
#include "ModuleRegistrar.h"

volatile bool quit;
EventProcessor *pEventProcessor;

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

void termHandler(int sig)
{
    quit = true;
    if (pEventProcessor)
    {
        pEventProcessor->stop();
    }
}

void getConfig(Dumais::JSON::JSON& jsonConfig)
{
    //TODO: this should be read from a file on disk
    jsonConfig.addObject("smtp");
    jsonConfig.addObject("web");
    jsonConfig.addObject("phone");
    jsonConfig.addObject("audio");
    jsonConfig.addObject("insteon");
    jsonConfig["insteon"].addValue(INSTEON_SERIAL_PORT,"serialport");
    jsonConfig["smtp"].addValue(SMTP_PORT,"bind");
    jsonConfig["web"].addValue(PASSWD_FILE,"passwd");
    jsonConfig["phone"].addValue(SOUNDS_FOLDER,"soundsfolder");
    jsonConfig["phone"].addValue(RTP_LOW_PORT,"rtplow");
    jsonConfig["phone"].addValue(RTP_HIGH_PORT,"rtphigh");
    jsonConfig["phone"].addValue(LOCAL_IP,"localip");
    jsonConfig["phone"].addValue(SIP_PORT,"sipport");
    jsonConfig["phone"].addValue(RONA_TIMEOUT,"ronatimeout");
    jsonConfig["audio"].addValue(SOUND_DEVICE,"sounddevice");
    jsonConfig["audio"].addValue(SOUNDS_FOLDER,"soundsfolder");
}

//For examples on how to use GPIO, check out the bcm2835 C library
int main(int argc, char** argv) 
{ 
    Logging::syslogServer = SYSLOG_SERVER;
    pEventProcessor = 0;
    quit = false;
    bool daemon = false;
    char *scriptName = 0;
    char *soundsFolder = 0;
    bool gendoc = false;

    for (int i=0;i<argc;i++)
    {
        std::string param = argv[i];
        if (param=="-d")
        {
            daemon = true;
        }
        else if (param=="-g")
        {
            gendoc = true;
            Logging::disabled = true;
        }
    }
    
    Dumais::JSON::JSON jsonConfig;
    getConfig(jsonConfig);
    ModuleProvider serviceProvider(jsonConfig);

    if (gendoc)
    {
        RESTInterface *pRESTInterface = new RESTInterface(&serviceProvider,0,0);
        pRESTInterface->init();
        std::string st = pRESTInterface->getAPI();
        printf("%s\r\n", st.c_str());
        return 0;
    }
    else if (daemon)
    {
        if (fork()>0) exit(0);
    }

    Logging::log("Starting\r\n");
    signal(SIGSEGV, handler);
    signal(SIGTERM, termHandler);
    Schedule schedule;
    WeatherHelper weather(LONGITUDE,LATITUDE);
    WebNotificationEngine webNotificationEngine;
    RESTInterface *pRESTInterface = new RESTInterface(&serviceProvider,&schedule,&weather);
    WebInterface *pWebInterface = new WebInterface(WEBSERVER_PORT,pRESTInterface,&webNotificationEngine);
    pWebInterface->configure(jsonConfig["web"]);

    pRESTInterface->init();

    // This needs to be created after services because it will load a script
    pEventProcessor = new EventProcessor(pRESTInterface,&schedule,&weather,&serviceProvider,SCRIPT_FILE);
    pEventProcessor->addEventListener(&webNotificationEngine);

    Logging::log("Starting Web Interface");
    pWebInterface->start();

    serviceProvider.startModules(pEventProcessor);
    pEventProcessor->reloadScript();

    Logging::log("=========== Ready ===========");
    while (pEventProcessor->timeSlice() && !quit);
    Logging::log("Exiting");
    Logging::log("Stopping Web Interface");
    pWebInterface->stop();
    serviceProvider.stopModules();

    delete pEventProcessor;
    Logging::log("=========== DHAS is now down ===========");
    return 0; 
}
