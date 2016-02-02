#ifndef RESTINTERFACE_H
#define RESTINTERFACE_H

#include <string>
#include "json/JSON.h"
#include "ModuleProvider.h"
#include "Schedule.h"
#include "WeatherHelper.h"
#include <pthread.h>
#include "ThreadSafeRestEngine.h"

class RESTInterface{
private:
    pthread_mutex_t mInvocationLock;
    ModuleProvider*    mpModuleProvider;
    Schedule* mpSchedule;
    WeatherHelper *mpWeatherHelper;

    ThreadSafeRestEngine mRESTEngine;

public:
	RESTInterface(ModuleProvider *pModuleProvider,Schedule* pSchedule,WeatherHelper *pWeather);
	~RESTInterface();

    void init();
    bool processQuery(Dumais::JSON::JSON& json, const std::string& query);

    void led_callback(RESTContext* context);
    void lcd_callback(RESTContext* context);
    void resetlcd_callback(RESTContext* context);
    void showevents_callback(RESTContext* context);
    void addevent_callback(RESTContext* context);
    void removeevent_callback(RESTContext* context);
    void gettime_callback(RESTContext* context);
    void help_callback(RESTContext* context);
    std::string getAPI();


};

#endif

