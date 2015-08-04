#include "CouchDB.h"
#include "HTTPCommunication.h"
#include "Logging.h"
#include "config.h"

#define MAXREQUESTS 100

void *ThreadStarter_couchdb(void *p)
{
    CouchDB* pc = (CouchDB*)p;
    pc->run();
}


CouchDB::CouchDB(std::string db)
{
    pthread_mutex_init(&mConditionLock,0);
    pthread_cond_init(&mWaitCondition,0);
    this->mpRequests = new MPSCRingBuffer<std::string>(MAXREQUESTS);
    this->mDb = db;
    this->mMustCompact = false;
    pthread_create(&mThreadHandle, 0, ThreadStarter_couchdb, this);
}

CouchDB::~CouchDB()
{
    this->mStopping = true;
    pthread_mutex_lock(&mConditionLock);
    pthread_cond_signal(&mWaitCondition);
    pthread_mutex_unlock(&mConditionLock);

    pthread_join(mThreadHandle,0);
    pthread_cond_destroy(&mWaitCondition);
    pthread_mutex_destroy(&mConditionLock);
}

void CouchDB::compact()
{
    pthread_mutex_lock(&mConditionLock);
    this->mMustCompact = true;
    pthread_cond_signal(&mWaitCondition);
    pthread_mutex_unlock(&mConditionLock);
}

void CouchDB::addDocument(Dumais::JSON::JSON& json)
{
    std::string st = json.stringify(false);
    if (!this->mpRequests->put(st))
    {
        Logging::log("Couchdb event queue overlofw. Dropping %s",st.c_str());
        return;
    }
    pthread_mutex_lock(&mConditionLock);
    pthread_cond_signal(&mWaitCondition);
    pthread_mutex_unlock(&mConditionLock);

}

bool CouchDB::unPoolDocument()
{
    std::string data;
    if (!this->mpRequests->get(data)) return false;

    std::string url = "/";
    url += this->mDb;

    bool ret = HTTPCommunication::postURL(COUCHDB_SERVER,url,data,"application/json",5984);
    if (ret)
    {
    }
    else
    {
        Logging::log("ERROR: Could not add document in CouchDB");
    }
    return true;
}

void CouchDB::run()
{
    this->mStopping = false;
    while (!this->mStopping)
    {
        bool compact = false;
        pthread_mutex_lock(&mConditionLock);
        pthread_cond_wait(&mWaitCondition,&mConditionLock);
        compact = this->mMustCompact;
        this->mMustCompact = false;
        pthread_mutex_unlock(&mConditionLock);
        while (unPoolDocument()); // unpool all documents until queue is empty

        if (compact)
        {
            std::string url = "/";
            url += this->mDb;
            url += "/";
            std::string st = HTTPCommunication::getURL(COUCHDB_SERVER,url,5984);
            Dumais::JSON::JSON j;
            j.parse(st);
            Logging::log("Will compact CouchDB. Size=%s",j["disk_size"].str().c_str());

            bool ret = HTTPCommunication::postURL(COUCHDB_SERVER,url+"_compact","","application/json",5984);
            if (ret)
            {
                std::string st = HTTPCommunication::getURL(COUCHDB_SERVER,url,5984);
                Dumais::JSON::JSON j;
                j.parse(st);
                Logging::log("CouchDB compacted. Size=%s",j["disk_size"].str().c_str());
            }
            else
            {
                Logging::log("ERROR: Could not compact CouchDB Database");
            }

        }
    }
}

