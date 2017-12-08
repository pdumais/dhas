#include "DHASLogging.h"
#include <iostream>
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
#include "EventLogger.h"
#include <fstream>
#include <streambuf>


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

void getConfig(Dumais::JSON::JSON& jsonConfig, const std::string& fname)
{

    std::ifstream ifs(fname);
    std::string data((std::istreambuf_iterator<char>(ifs)),std::istreambuf_iterator<char>());
    jsonConfig.parse(data);
    //TODO: this should be read from a file on disk
/*    jsonConfig.addObject("web");
    jsonConfig.addObject("phone");
    jsonConfig.addObject("audio");
    jsonConfig.addObject("insteon");
    jsonConfig["insteon"].addValue(INSTEON_SERIAL_PORT,"serialport");
    jsonConfig["insteon"].addValue(INSTEON_IP,"ip");
    jsonConfig["insteon"].addValue(INSTEON_IP_PORT,"port");
    jsonConfig["web"].addValue(PASSWD_FILE,"passwd");
    jsonConfig["web"].addValue("/etc/httpd/conf/ssl.key/dumaisnet.key","ssl_key");
    jsonConfig["web"].addValue("/etc/httpd/conf/ssl.crt/dumaisnet.crt","ssl_cert");
    jsonConfig["phone"].addValue(SOUNDS_FOLDER,"soundsfolder");
    jsonConfig["phone"].addValue(RTP_LOW_PORT,"rtplow");
    jsonConfig["phone"].addValue(RTP_HIGH_PORT,"rtphigh");
    jsonConfig["phone"].addValue(LOCAL_IP,"localip");
    jsonConfig["phone"].addValue(SIP_PORT,"sipport");
    jsonConfig["phone"].addValue(RONA_TIMEOUT,"ronatimeout");
    jsonConfig["audio"].addValue(SOUND_DEVICE,"sounddevice");
    jsonConfig["audio"].addValue(SOUNDS_FOLDER,"soundsfolder");*/
}


unsigned int getServerPID()
{
    unsigned int pid;
    std::fstream pidfile("/var/run/homeautomation.pid",std::ios_base::in);
    if (pidfile.is_open())
    {
        if (pidfile >> pid)
        {
            return pid;
        }
    }

    return 0;
}

void forkprocess()
{
    unsigned int pid = getServerPID();
    if (pid != 0)
    {
        std::stringstream ss;
        ss << "/proc/"<<pid<<"/cmdline";
        std::fstream procfile(ss.str(),std::ios_base::in);
        std::string cmdline;
        if (procfile >> cmdline)
        {
            if (cmdline.find("homeautomation") != std::string::npos)
            {
                printf("Home automation server is already running\r\n");
                exit(-1);
            }
            else
            {
                LOG("WARNING: stale pid file contains pid [" << pid << "] for process ["<< cmdline.c_str() << "]");
            }
        } 
    }
    if (fork()>0) exit(0);

}

//For examples on how to use GPIO, check out the bcm2835 C library
int main(int argc, char** argv) 
{ 
    Dumais::JSON::JSON jsonConfig;
    getConfig(jsonConfig,"./config.json");

    Dumais::Utils::Logging::logger = new SyslogLogging(jsonConfig["syslog"]["host"].str());

    pEventProcessor = 0;
    quit = false;
    bool daemon = false;
    char *scriptName = 0;
    char *soundsFolder = 0;
    bool gendoc = false;

    if (argc < 2)
    {
        printf("Usage: homeautomation -d|-g|-r\r\n");
        printf("       -d launch daemon\r\n");
        printf("       -g generate API doc\r\n");
        printf("       -r send SIGHUP to daemon\r\n");
        printf("       -k send SIGTERM to daemon\r\n");
        printf("       -c run normal mode\r\n");
        exit(0);
    }

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
            delete Dumais::Utils::Logging::logger;
            Dumais::Utils::Logging::logger = 0;
        }
        else if (param=="-r")
        {
            unsigned int pid = getServerPID();
            if (pid == 0)
            {
                printf("Server is not running\r\n");
                exit(-1);
            }
            printf("Sending SIGHUP to server process [%i]\r\n",pid);
            kill(pid,SIGHUP);
            exit(0);
        }
        else if (param=="-k")
        {
            unsigned int pid = getServerPID();
            if (pid == 0)
            {
                printf("Server is not running\r\n");
                exit(-1);
            }
            printf("Sending SIGTERM to server process [%i]\r\n",pid);
            kill(pid,SIGTERM);
            exit(0);
        }
    }
    
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
        forkprocess();
    }

    std::fstream pidfile("/var/run/homeautomation.pid",std::ios_base::out);
    pidfile << getpid();
    pidfile.close();

    LOG("Starting");
    signal(SIGSEGV, handler);
    signal(SIGTERM, termHandler);
    Schedule schedule(jsonConfig["schedule"]);
    WeatherHelper weather(jsonConfig["location"]["longitude"].toDouble(),jsonConfig["location"]["latitude"].toDouble());
    WebNotificationEngine webNotificationEngine;
    RESTInterface *pRESTInterface = new RESTInterface(&serviceProvider,&schedule,&weather);
    WebInterface *pWebInterface = new WebInterface(jsonConfig["web"]["port"].toInt(),pRESTInterface,&webNotificationEngine);
    pWebInterface->configure(jsonConfig["web"]);

    pRESTInterface->init();

    // setup couchDB views at first
    Mysql *mysql = new Mysql("dhas",jsonConfig["database"]["host"].str(),3306,jsonConfig["database"]["user"].str(), jsonConfig["database"]["password"].str()); 

    EventLogger eventLogger(mysql);

    // This needs to be created after services because it will load a script
    pEventProcessor = new EventProcessor(pRESTInterface,&schedule,&weather,&serviceProvider,jsonConfig["script"]["path"].str());
    pEventProcessor->addEventListener(&eventLogger);
    pEventProcessor->addEventListener(&webNotificationEngine);

    LOG("Starting Web Interface");
    pWebInterface->start();

    serviceProvider.startModules(pEventProcessor);
    pEventProcessor->reloadScript();

    LOG("=========== Ready ===========");
    while (pEventProcessor->timeSlice() && !quit) sleep(1);
    LOG("Exiting");
    LOG("Stopping Web Interface");
    pWebInterface->stop();
    serviceProvider.stopModules();

    delete pEventProcessor;
    delete mysql;
    LOG("=========== DHAS is now down ===========");

    return 0; 
}
