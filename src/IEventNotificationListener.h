#pragma once
#include <string>
#include "json/JSON.h"

class IEventNotificationListener
{
public:
    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent)=0;
};
