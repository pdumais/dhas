#include "Logging.h"
//#include "config.h"
#include "EventProcessor.h"
#include "LUAEngine.h"
#include "JSEngine.h"
#include "stdio.h"
#include <vector>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>

typedef void (*sighandler)(int);

static EventProcessor *spEventProcessor;

void onAlarm(int sig)
{
    time_t t;
    time(&t);
    Dumais::JSON::JSON json;
    json.addValue("timer","event");
    json.addValue((unsigned int)t,"timestamp");
    spEventProcessor->processSchedule(t);
    spEventProcessor->appendPeriodicData(json);
    spEventProcessor->processEvent(json);
    spEventProcessor->checkSun(t);

}

void onHUP(int sig)
{
    spEventProcessor->scheduleScriptReload();
}


EventProcessor::EventProcessor(RESTInterface *p,Schedule *pSchedule, 
                               WeatherHelper *pWeather, ModuleProvider *sp,
                               const std::string& scriptFile)
{
    this->mScriptFile = scriptFile;
    pthread_mutex_init(&mMessageQueueLock,0);
    pthread_cond_init(&mMessageQueueWaitCondition,0);

    mpModuleProvider = sp;
    spEventProcessor = this;
    mpWeatherHelper = pWeather;
    mpRESTInterface = p;
    mpScriptEngine = 0;
    mpSchedule = pSchedule;
    mpCouchDB = new CouchDB("dhas");

    time(&this->mLastCouchCompact);
    
    struct itimerval timer;
    time_t t;
    time(&t);
    timer.it_value.tv_sec = 60-(t%60);
    timer.it_value.tv_usec =0;
    timer.it_interval.tv_sec=60;
    timer.it_interval.tv_usec=0;

    setitimer(ITIMER_REAL,&timer,0);
    signal(SIGALRM, (sighandler)onAlarm);
    signal(SIGHUP, (sighandler)onHUP);

}

EventProcessor::~EventProcessor(){
    pthread_mutex_destroy(&mMessageQueueLock);
    pthread_cond_destroy(&mMessageQueueWaitCondition);
    removeEventListener(mpScriptEngine);
    delete mpCouchDB;
    delete mpScriptEngine;
}

void EventProcessor::checkSun(time_t t)
{
    time_t sunset = mpWeatherHelper->getSunSet();
    time_t sunrise = mpWeatherHelper->getSunRise();
    if (sunrise >= t && sunrise < (t+60)) {
        Dumais::JSON::JSON json;
        json.addValue("sunrise","event");
        json.addValue((unsigned int)t,"timestamp");
        spEventProcessor->processEvent(json);
        Logging::log("sunrise at %i (%i)",t,sunrise);
    } else if (sunset >= t && sunset < (t+60)) {
        Dumais::JSON::JSON json;
        json.addValue("sunset","event");
        json.addValue((unsigned int)t,"timestamp");
        spEventProcessor->processEvent(json);
        Logging::log("sunset at %i (%i)",t,sunset);
    }

}

void EventProcessor::addEventListener(IEventNotificationListener* pl)
{
    mEventListeners.push_back(pl);
}

void EventProcessor::removeEventListener(IEventNotificationListener* pl)
{
    for (std::vector<IEventNotificationListener*>::iterator it = mEventListeners.begin(); it != mEventListeners.end(); it++)
    {
        if (*it == pl)
        {
            mEventListeners.erase(it);
            return;
        }
    }
}

void EventProcessor::scheduleScriptReload()
{
    Logging::log("Scheduling script reload");
    mNeedReloadScript = true;
    pthread_cond_signal(&mMessageQueueWaitCondition);
}

void EventProcessor::stop()
{
    pthread_cond_signal(&mMessageQueueWaitCondition);
}

void EventProcessor::reloadScript()
{
    Logging::log("Reloading script");
//    mpModuleProvider->suspendModules();
    if (mpScriptEngine!=0)
    {
        removeEventListener(mpScriptEngine);
        delete mpScriptEngine;
    }

#ifdef SCRIPTLUA    
    mpScriptEngine = new LUAEngine();
#elif SCRIPTJS
    mpScriptEngine = new JSEngine();
#else
    #error No valid script engine defined
#endif
    mpScriptEngine->setRESTInterface(mpRESTInterface);
    mpScriptEngine->load(this->mScriptFile);
    addEventListener(mpScriptEngine);
    mNeedReloadScript = false;    
//    mpModuleProvider->resumeModules();
}

void EventProcessor::processEvent(Dumais::JSON::JSON &message)
{
    processEvent(message.stringify(false));

    time_t t;
    time(&t);
    message.addValue((unsigned int)t,"timestamp");
    mpCouchDB->addDocument(message);
}

void EventProcessor::processEvent(std::string str)
{

    // we need to lock this since other service might want to add something. And main thread might
    // want to pull something out
    pthread_mutex_lock(&mMessageQueueLock);
    mMessageQueue.push(str);
    pthread_cond_signal(&mMessageQueueWaitCondition);
    pthread_mutex_unlock(&mMessageQueueLock);
}

bool EventProcessor::timeSlice()
{
    std::vector<std::string> tmpMessages;    

    // We lock the message queue because other services might try to add something to it. And then
    // we use a wait condition to wait until at lwast one event gets added. This is to avoid
    // spinning for nothing.
    pthread_mutex_lock(&mMessageQueueLock);
    pthread_cond_wait(&mMessageQueueWaitCondition,&mMessageQueueLock);

    if (mNeedReloadScript)
    {       
        reloadScript();
    }

    while (mMessageQueue.size()>0)
    {
        tmpMessages.push_back(mMessageQueue.front());
        mMessageQueue.pop();
    }
    pthread_mutex_unlock(&mMessageQueueLock);

    for (std::vector<std::string>::iterator it = tmpMessages.begin();it!=tmpMessages.end();it++)
    {
        for (std::vector<IEventNotificationListener*>::iterator it2 = mEventListeners.begin();it2!=mEventListeners.end();it2++)
        {
            (*it2)->notifyEvent(*it);
        }
    }
    return true;
}

void EventProcessor::processSchedule(time_t t)
{
    // This will be called every minute. compact database if its been 24hs
    if (t> (this->mLastCouchCompact+(24*60*60)))
    {
        this->mLastCouchCompact = t;        
        this->mpCouchDB->compact();      
    } 

    std::vector<ScheduledEvent> list = mpSchedule->getDueEvents(t);
    for (std::vector<ScheduledEvent>::iterator it = list.begin();it!=list.end();it++)
    {
        ScheduledEvent event = *it;
        Dumais::JSON::JSON json;
        event.toJSON(json);
        processEvent(json.stringify(false));
        mpSchedule->removeEvent(event.getID());
    }
  
}
void EventProcessor::appendPeriodicData(Dumais::JSON::JSON& json)
{
    this->mpModuleProvider->appendPeriodicData(json);
}