#pragma once
#include <string>
#include "JSON.h"

class IEventNotificationListener
{
public:
    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent)=0;
};
