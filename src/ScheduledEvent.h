#ifndef SCHEDULEDEVENT_H
#define SCHEDULEDEVENT_H
#define HAVE_CXX_STDHEADERS
#include <db_cxx.h>
#include "json/JSON.h"

class ScheduledEvent{
private:
    std::string mName;
    std::string mParam;
    int mMinute;
    int mHour;
    bool mRecurent;
    time_t mTimeout;
    int mID;

public:
    ScheduledEvent(std::string name, std::string param, int min, int hour);
    ScheduledEvent(Dumais::JSON::JSON& json);    
    ScheduledEvent(const ScheduledEvent& event);  

    ~ScheduledEvent();

    int getID();
  bool isDue(time_t t);
    time_t getTimeout();
  const std::string& getName();
  const std::string& getParam();

    int getMinute();
    int getHour();

    void toJSON(Dumais::JSON::JSON& json);

};

#endif

