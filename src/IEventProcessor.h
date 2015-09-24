#pragma once
#include "json/JSON.h"

class IEventProcessor{
public:
    virtual void stop() = 0;
    virtual void processEvent(Dumais::JSON::JSON &message) = 0;
    virtual void processSchedule(time_t t) = 0;
    virtual void scheduleScriptReload() = 0;
    virtual void checkSun(time_t t) = 0;
    virtual void appendPeriodicData(Dumais::JSON::JSON& json) = 0;
};


