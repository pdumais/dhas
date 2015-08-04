#pragma once
#include <string>
#include "JSON.h"
#include <pthread.h>
#include "MPSCRingBuffer.h"

class CouchDB
{
private:
    bool unPoolDocument();

    pthread_t   mThreadHandle;
    volatile bool        mStopping; 
    bool                 mMustCompact;
    MPSCRingBuffer<std::string>* mpRequests;
    std::string mDb;
    pthread_mutex_t mConditionLock;
    pthread_cond_t mWaitCondition;

public:
    CouchDB(std::string db);
    ~CouchDB();

    void compact();
    void addDocument(Dumais::JSON::JSON& json);
    void run();
};
