#include "DHASLogging.h"
#include "EventLogger.h"

EventLogger::EventLogger(CouchDB *pCouchDB)
{
    this->mpCouchDB = pCouchDB;
    time(&this->mLastCouchCompact);
}

void EventLogger::notifyEvent(const Dumais::JSON::JSON& jsonEvent)
{
    time_t t;
    time(&t);
    //jsonEvent.addValue((unsigned int)t,"timestamp");
    mpCouchDB->addDocument(jsonEvent);

    if (t> (this->mLastCouchCompact+(24*60*60)))
    {
        this->mLastCouchCompact = t;
        this->mpCouchDB->compact();
    }

}
