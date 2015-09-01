#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include "IEventProcessor.h"
#include "RESTInterface.h"
#include "IScriptEngine.h"
#include <queue>
#include <vector>
#include "Schedule.h"
#include "WeatherHelper.h"
#include "WebNotificationEngine.h"

class EventProcessor : public IEventProcessor{
private:
    RESTInterface *mpRESTInterface;
    IScriptEngine *mpScriptEngine;
    std::queue<Dumais::JSON::JSON> mMessageQueue;
    pthread_mutex_t mMessageQueueLock;
    pthread_cond_t mMessageQueueWaitCondition;
    Schedule* mpSchedule;
    WeatherHelper* mpWeatherHelper;
    ModuleProvider *mpModuleProvider;
    std::vector<IEventNotificationListener*> mEventListeners;
    std::string mScriptFile;

    bool mNeedReloadScript;

public:
	EventProcessor(RESTInterface *p, Schedule *pSchedule, WeatherHelper *pWeather, ModuleProvider *sp, const std::string& scriptFile);
	~EventProcessor();
    
    void stop();
    void processEvent(Dumais::JSON::JSON &message);
    void processSchedule(time_t t);
    bool timeSlice(); // will return false when it is time to shutdown the application
    void scheduleScriptReload();
    void checkSun(time_t t);
    void appendPeriodicData(Dumais::JSON::JSON& json);

    void addEventListener(IEventNotificationListener* pl);
    void removeEventListener(IEventNotificationListener* pl);

    void reloadScript();
    
};

#endif

