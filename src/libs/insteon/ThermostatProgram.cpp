#include "utils/Logging.h"
#include "ThermostatProgram.h"
#include "config.h"
#include <string.h>
#include <sstream>
#include <stdlib.h>

#define KEYPREFIX "absolutehour-"

ThermostatProgram::ThermostatProgram()
{
    pthread_mutex_init(&mListLock,0);
    db_create(&mpDB,0,0);
    mpDB->open(mpDB,0,THERMOSTATPROGRAMDB,"",DB_HASH,DB_CREATE,664); 
   
    DBC *pCursor;
    mpDB->cursor(mpDB,0,&pCursor,0);
    mActive = false;
    DBT k,data;
    memset(&k, 0, sizeof(k));
    memset(&data, 0, sizeof(data));

    char tmp[255];

    while (pCursor->c_get(pCursor,&k,&data,DB_NEXT)==0)
    {
        memcpy(tmp,k.data,k.size); tmp[k.size]=0;        
        std::string hourString = tmp;
        memcpy(tmp,data.data,data.size); tmp[data.size]=0;
        std::string str = tmp;
        if (hourString == "active")
        {
            if (str=="true") mActive = true;
        }
        else 
        {
            LOG("ThermostatProgram: Key ["<< hourString.c_str()<<"], val ["<< str.c_str()<<"]");
            hourString = hourString.substr(strlen(KEYPREFIX));
            Dumais::JSON::JSON json;
            json.parse(str);
            ProgramPoint point;
            unsigned int hour = atoi(hourString.c_str());
            point.mode = atoi(json["mode"].str().c_str()); 
            point.temp = atoi(json["temp"].str().c_str());
            mEventList[hour]=point;
        }

        memset(&k, 0, sizeof(k));
        memset(&data, 0, sizeof(data));

    }

    pCursor->c_close(pCursor);
    LOG("ThermostatProgram database opened");
   
}

ThermostatProgram::~ThermostatProgram()
{
    pthread_mutex_destroy(&mListLock);
    mpDB->close(mpDB,0);
}

bool ThermostatProgram::isActive()
{
    return mActive;
}

void ThermostatProgram::setActive(bool active)
{
    mActive = active;
    std::string key = "active";
    std::string str = "false";
    if (active) str = "true";

    DBT k,data;
    memset(&k, 0, sizeof(k));
    memset(&data, 0, sizeof(data));
    k.data = (char*)key.c_str(); k.size = key.size()+1;
    data.data = (char*)str.c_str(); data.size = str.size()+1;

    mpDB->put(mpDB,0,&k,&data,0);
    mpDB->sync(mpDB,0);
}

void ThermostatProgram::setPoint(unsigned char absoluteTime, unsigned char temp, unsigned char mode)
{
    ProgramPoint point;
    point.temp = temp;
    point.mode = mode;
    Dumais::JSON::JSON json;
    json.addValue(temp,"temp");
    json.addValue(mode,"mode");
    std::string str = json.stringify(false);

    unsigned int aTime = absoluteTime;

    std::stringstream ss;
    ss << KEYPREFIX << aTime;
    std::string key = ss.str();

    const char* ck = key.c_str();
    const char* cd = str.c_str();

    DBT k,data;
    memset(&k, 0, sizeof(k));
    memset(&data, 0, sizeof(data));
    k.data = (char*)ck; k.size = strlen(ck);
    data.data = (char*)cd; data.size = strlen(cd);;

    LOG("Adding thermostat program point at "<<ck<<": "<<cd);

    pthread_mutex_lock(&mListLock);
    mEventList[absoluteTime] = point;
    mpDB->put(mpDB,0,&k,&data,0);
    mpDB->sync(mpDB,0); // not very good when using flash memeory, but this is the best way to make sure we keep data after crashing
    pthread_mutex_unlock(&mListLock);

}

ProgramPoint ThermostatProgram::getPoint(unsigned char absoluteTime)
{
    ProgramPoint point;
    pthread_mutex_lock(&mListLock);
    if (mEventList.find(absoluteTime) != mEventList.end())
    {
        point = mEventList[absoluteTime];
    }
    else
    {
        point.mode = 0;
    }
    pthread_mutex_unlock(&mListLock);
    return point;
}

void ThermostatProgram::removePoint(unsigned char hour)
{
    pthread_mutex_lock(&mListLock);
    std::map<int,ProgramPoint>::iterator it = mEventList.find(hour);
    if (it!=mEventList.end())
    {
        mEventList.erase(it);
    } else {
        pthread_mutex_unlock(&mListLock);
        return;
    }

    unsigned int aTime = hour;
    std::stringstream ss;
    ss << KEYPREFIX << aTime;
    std::string key = ss.str();
    const char* ck = key.c_str();

    DBT k;
    memset(&k, 0, sizeof(k));
    k.data = (char*)ck; k.size = strlen(ck);
    mpDB->del(mpDB,0,&k,0);
    mpDB->sync(mpDB,0); // not very good when using flash memeory, but this is the best way to make sure we keep data after crashing
    pthread_mutex_unlock(&mListLock);

}

