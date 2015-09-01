#pragma once

#include "CouchDB.h"
#include "IEventNotificationListener.h"

class EventLogger: public IEventNotificationListener
{
private:
    CouchDB* mpCouchDB;
    time_t mLastCouchCompact;

public:
    EventLogger(CouchDB *pCouchDB);
    
    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent);
}; 
