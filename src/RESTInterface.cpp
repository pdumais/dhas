#include "Logging.h"
#include "RESTInterface.h"
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>
#include <functional>

using namespace Dumais::JSON;
using namespace std;

RESTInterface::RESTInterface(ModuleProvider *pModuleProvider,Schedule* pSchedule,WeatherHelper *pWeather)
{
    pthread_mutex_init(&mInvocationLock,0);
    mpWeatherHelper = pWeather;
    mpSchedule = pSchedule;
    mpModuleProvider = pModuleProvider;
}

RESTInterface::~RESTInterface()
{
    pthread_mutex_destroy(&mInvocationLock);
}

void RESTInterface::init()
{
	RESTCallBack *p;

	p = new RESTCallBack(this,&RESTInterface::showevents_callback,"show all scheduled events");
	mRESTEngine.addCallBack("/events/show","GET",p);

	p = new RESTCallBack(this,&RESTInterface::addevent_callback,"add a scheduled event. This event will trigger the LUA script at the defined time");
	p->addParam("name","Event name");
	p->addParam("p","user defined data");
	p->addParam("min","minute of the hour at which to trigger the event");
	p->addParam("hour","hour of the day at which to trigger the event");
	mRESTEngine.addCallBack("/events/add","GET",p);

	p = new RESTCallBack(this,&RESTInterface::removeevent_callback,"remove a scheduled event");
	p->addParam("id","event ID");
	mRESTEngine.addCallBack("/events/remove","GET",p);

	p = new RESTCallBack(this,&RESTInterface::gettime_callback,"get current time");
	mRESTEngine.addCallBack("/events/gettime","GET",p);

	p = new RESTCallBack(this,&RESTInterface::help_callback,"display API documentation");
	mRESTEngine.addCallBack("/help","GET",p);

    mpModuleProvider->registerCallBacks(&mRESTEngine);

}	






bool RESTInterface::processQuery(Dumais::JSON::JSON& json, const std::string& query)
{
    /*
     * We want to lock here because the Webserver thread and the EventProcessor thread make
     * calls to this function. We want to make sure that only one service call at a time is made.
     *
     * This creates a bottleneck though. If several webserver threads and the eventProcessor wants
     * to execute a command, there will be only one at a time that will work in here.
     * But that's OK. We don't need performance here, we need reliability. This way of doing things
     * will guarantee a proper sequence of events
     */
    pthread_mutex_lock(&mInvocationLock);
    bool b = (mRESTEngine.invoke(json,query,"GET","")==RESTEngine::OK);
    pthread_mutex_unlock(&mInvocationLock);

    return b;
}


/*void RESTInterface::temperature_callback(Dumais::JSON::JSON& json, RESTParameters *p)
{
  int temp = mpWeatherHelper->getCurrentTemperature();
  json.addValue(temp,"temperature");
}*/

void RESTInterface::showevents_callback(RESTContext* context)
{
  Dumais::JSON::JSON& json = context->returnData;
  json.addList("events");
  std::vector<ScheduledEvent> list = mpSchedule->getAllEvents();
  for (std::vector<ScheduledEvent>::iterator it= list.begin();it!=list.end();it++)
  {
	  ScheduledEvent event = *it;
	  Dumais::JSON::JSON& obj = json["events"].addObject("scheduledevent");
	  obj.addValue(event.getID(),"id");
	  obj.addValue(event.getName(),"name");
	  obj.addValue(event.getParam(),"param");
	  obj.addValue(event.getMinute(),"minute");
	  obj.addValue(event.getHour(),"hour");
	  obj.addValue((unsigned int)event.getTimeout(),"timeout");
	  
  }
}

void RESTInterface::addevent_callback(RESTContext* context)
{
    RESTParameters* p = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    ScheduledEvent *pEvent = new ScheduledEvent(p->getParam("name"),p->getParam("p"),atoi(p->getParam("min").c_str()),atoi(p->getParam("hour").c_str()));
    mpSchedule->addEvent(pEvent);
}

void RESTInterface::removeevent_callback(RESTContext* context)
{
    RESTParameters* p = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    int id = atoi(p->getParam("id").c_str());
    mpSchedule->removeEvent(id);                
}

void RESTInterface::gettime_callback(RESTContext* context)
{
    Dumais::JSON::JSON& json = context->returnData;
  time_t t;
  time(&t);
  time_t sunrise = mpWeatherHelper->getSunRise();
  time_t sunset = mpWeatherHelper->getSunSet();
  bool night = false;
  if (t>=sunset || t<sunrise) night = true;

  json.addValue((unsigned int)t,"timestamp");
  json.addValue(night?"true":"false","night");
}

void RESTInterface::help_callback(RESTContext* context)
{
    Dumais::JSON::JSON& json = context->returnData;
    mRESTEngine.documentInterface(json);
}

std::string RESTInterface::getAPI()
{
    Dumais::JSON::JSON json;
    mRESTEngine.documentInterface(json);
    return json.stringify(true);
}

