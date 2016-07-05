#include "DHASLogging.h"
#include "EventLogger.h"

EventLogger::EventLogger(Mysql* pMysql)
{
    this->mpMysql = pMysql;
}

void EventLogger::notifyEvent(const Dumais::JSON::JSON& jsonEvent)
{
    time_t t;
    mpMysql->addDocument(jsonEvent);
}
