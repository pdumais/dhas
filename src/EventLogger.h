#pragma once

#include "Mysql.h"
#include "IEventNotificationListener.h"

class EventLogger: public IEventNotificationListener
{
private:
    Mysql* mpMysql;

public:
    EventLogger(Mysql* pMysql);
    
    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent);
}; 
