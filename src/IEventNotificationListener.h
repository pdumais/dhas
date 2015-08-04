#pragma once
#include <string>

class IEventNotificationListener
{
public:
    virtual void notifyEvent(const std::string& jsonEvent)=0;
};
