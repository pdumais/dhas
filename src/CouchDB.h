#pragma once
#include <string>
#include "JSON.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "MPSCRingBuffer.h"

class CouchDB
{
private:
    bool unPoolDocument();

    std::thread   mThreadHandle;
    volatile bool        mStopping; 
    volatile bool        mMustCompact;
    MPSCRingBuffer<std::string>* mpRequests;
    std::string mDb;
    std::mutex mConditionLock;
    std::condition_variable_any mWaitCondition;
    std::string mServer;
    int mPort;

public:
    CouchDB(const std::string& db, const std::string& server, int port);
    ~CouchDB();

    void compact();
    void addDocument(const Dumais::JSON::JSON& json);
    void run();

    void createDb();
    void createViewsIfNonExistant(std::string views);
};
