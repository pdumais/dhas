#include "ScheduledEvent.h"
#include <stdlib.h>

ScheduledEvent::ScheduledEvent(std::string name, std::string param, int min, int hour){
  mName = name;
  mParam = param;
  mMinute = min;
  mHour = hour;

    time_t t;
    time(&t);
    //TODO: this is dangerous. Could be a collision
    mID = ((int)this) + (int)t;

    struct tm *ltm = localtime(&t);
    struct tm tm2;

    tm2.tm_sec = 0;
    tm2.tm_min = mMinute;
    tm2.tm_hour = mHour;
    tm2.tm_mday= ltm->tm_mday;
    tm2.tm_mon=ltm->tm_mon;
    tm2.tm_year=ltm->tm_year;
    tm2.tm_isdst=-1;
    // If time is already elapsed for today, then set it for tomorow
    time_t t2 = mktime(&tm2);
    if (t2>t)
    {
        mTimeout = t2;
    } else {
        // already elapsed today, so we will set it for tomorow at the same time
        //TODO: what if it crosses a DST boundary? we should detect that and add or remove 1h
        mTimeout = t2+(60*60*24);
    }
}

ScheduledEvent::ScheduledEvent(Dumais::JSON::JSON& json)
{
    mName = json["name"].str();
    mParam = json["param"].str();
    mID = atoi(json["id"].str().c_str());
    mMinute = atoi(json["min"].str().c_str());
    mHour = atoi(json["hour"].str().c_str());
    mTimeout = atoi(json["timeout"].str().c_str());
}

ScheduledEvent::ScheduledEvent(const ScheduledEvent& event)
{
    if (&event != this)
    {
        mName = event.mName;
        mParam = event.mParam;
        mID = event.mID;
        mMinute = event.mMinute;
        mHour = event.mHour;
        mTimeout = event.mTimeout;
    }

}


ScheduledEvent::~ScheduledEvent(){
}

bool ScheduledEvent::isDue(time_t t)
{

    if (t>=mTimeout)
    {
        return true;
    }

/*  struct tm *ltm = localtime(&t);
  if (ltm->tm_hour>=mHour)
  {
    if (ltm->tm_min>=mMinute)
    {
      if (t>=(mLastCheck+(60*60*24))) // last time we checked was at least 24h ago. So it is not the same hour:min
      {
        mLastCheck = t;
        return true;
      }
    }
  }*/
  return false;
}

int ScheduledEvent::getMinute()
{
    return mMinute;
}

int ScheduledEvent::getHour()
{
    return mHour;
}

time_t ScheduledEvent::getTimeout()
{
    return mTimeout;
}

const std::string& ScheduledEvent::getName()
{
  return mName;
}

const std::string& ScheduledEvent::getParam()
{
  return mParam;
}

int ScheduledEvent::getID()
{
    return mID;
}

void ScheduledEvent::toJSON(Dumais::JSON::JSON& json)
{
    json.addValue("scheduledevent","event");
    json.addValue((int)mID,"id");
    json.addValue(mName,"name");
    json.addValue(mParam,"param");
    json.addValue(mMinute,"min");
    json.addValue(mHour,"hour");
    json.addValue((unsigned int)mTimeout,"timeout");
}
