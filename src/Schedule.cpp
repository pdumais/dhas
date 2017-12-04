#include "DHASLogging.h"
#include "Schedule.h"
#include <string.h>
#include <sstream>

Schedule::Schedule(Dumais::JSON::JSON& json)
{
    pthread_mutex_init(&mListLock,0);
    db_create(&mpDB,0,0);
    int ret = mpDB->open(mpDB,0,json["db"].str().c_str(),"",DB_HASH,DB_CREATE,664); 

   //TODO: read all
    DBC *pCursor;
    mpDB->cursor(mpDB,0,&pCursor,0);
    DBT k,data;
    memset(&k, 0, sizeof(k));
    memset(&data, 0, sizeof(data));
    while (pCursor->c_get(pCursor,&k,&data,DB_NEXT)==0)
    {
        std::string str = (char*)data.data;
        Dumais::JSON::JSON json;
        json.parse(str);
        ScheduledEvent *pEvent = new ScheduledEvent(json);
        mEventList[pEvent->getID()]=pEvent;
    }

    pCursor->c_close(pCursor);
    LOG("Schedule database opened");
   
}

Schedule::~Schedule()
{
    pthread_mutex_destroy(&mListLock);
    mpDB->close(mpDB,0);
}

void Schedule::addEvent(ScheduledEvent* pEvent)
{
    pthread_mutex_lock(&mListLock);
    mEventList[pEvent->getID()] = pEvent;
    Dumais::JSON::JSON json;
    pEvent->toJSON(json);
    std::string str = json.stringify(false);
    
    std::stringstream ss;
    ss<<pEvent->getID();
    DBT k,data;
    memset(&k, 0, sizeof(k));
    memset(&data, 0, sizeof(data));
    k.data = (char*)ss.str().c_str(); k.size = ss.str().size();//+1;
    data.data = (char*)str.c_str(); data.size = str.size();//+1;
    mpDB->put(mpDB,0,&k,&data,0);
    mpDB->sync(mpDB,0); // not very good when using flash memeory, but this is the best way to make sure we keep data after crashing
    pthread_mutex_unlock(&mListLock);

}

void Schedule::removeEvent(int id)
{

    pthread_mutex_lock(&mListLock);
    ScheduledEvent *pEvent = 0;
    std::map<int,ScheduledEvent*>::iterator it = mEventList.find(id);
    if (it!=mEventList.end())
    {
        pEvent = mEventList[id];
        mEventList.erase(it);
    } else {
        pthread_mutex_unlock(&mListLock);
        return;
    }

    std::stringstream ss;
    ss<<id;
    DBT k;
    memset(&k, 0, sizeof(k));
    k.data = (char*)ss.str().c_str(); k.size = ss.str().size();//+1;
    mpDB->del(mpDB,0,&k,0);
    mpDB->sync(mpDB,0); // not very good when using flash memeory, but this is the best way to make sure we keep data after crashing
    delete pEvent;
    pthread_mutex_unlock(&mListLock);
}

std::vector<ScheduledEvent> Schedule::getDueEvents(time_t t)
{
    std::vector<ScheduledEvent> retList;
  
    pthread_mutex_lock(&mListLock);
    for (std::map<int,ScheduledEvent*>::iterator it = mEventList.begin();it!=mEventList.end();it++)
    {
        ScheduledEvent* pEvent = it->second;
        if (pEvent->isDue(t))
        {
            retList.push_back(*pEvent);
        }
    }
    pthread_mutex_unlock(&mListLock);
    return retList;
}

std::vector<ScheduledEvent> Schedule::getAllEvents()
{
    int r = pthread_mutex_lock(&mListLock);

    std::vector<ScheduledEvent> retList;
    for (std::map<int,ScheduledEvent*>::iterator it = mEventList.begin();it!=mEventList.end();it++)
    {
        retList.push_back(*(it->second));
    }
    pthread_mutex_unlock(&mListLock);

    return retList;
}
