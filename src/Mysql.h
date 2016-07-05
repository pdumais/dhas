#pragma once
#include <mysql.h>
#include <string>
#include "json/JSON.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "utils/MPSCRingBuffer.h"

class Mysql
{
private:
    bool unPoolDocument();
    void createDb();

    MYSQL* mpMysql;
    std::thread   mThreadHandle;
    volatile bool        mStopping; 
    Dumais::Utils::MPSCRingBuffer<std::string>* mpRequests;
    std::string mDb;
    std::mutex mConditionLock;
    std::condition_variable_any mWaitCondition;
    std::string mServer;
    std::string mUser;
    std::string mPassword;
    int mPort;

public:
    Mysql(const std::string& db, const std::string& server, int port, const std::string& username, const std::string& password);
    ~Mysql();

    void addDocument(const Dumais::JSON::JSON& json);
    void run();

};
