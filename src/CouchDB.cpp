#include "CouchDB.h"
#include "HTTPCommunication.h"
#include "Logging.h"

#define MAXREQUESTS 100

CouchDB::CouchDB(const std::string& db, const std::string& server, int port)
{
    mServer = server;
    mPort = port;
    this->mpRequests = new MPSCRingBuffer<std::string>(MAXREQUESTS);
    this->mDb = db;
    this->mMustCompact = false;
    
    mThreadHandle = std::thread([this](){
        this->run();
    });
}

CouchDB::~CouchDB()
{
    this->mStopping = true;
    
    mConditionLock.lock();
    mWaitCondition.notify_one();
    mConditionLock.unlock();

    if (mThreadHandle.joinable()) mThreadHandle.join();
}

void CouchDB::compact()
{
    mConditionLock.lock();
    this->mMustCompact = true;
    mWaitCondition.notify_one();
    mConditionLock.unlock();
}

void CouchDB::addDocument(const Dumais::JSON::JSON& json)
{
    std::string st = json.stringify(false);
    if (!this->mpRequests->put(st))
    {
        Logging::log("Couchdb event queue overlofw. Dropping %s",st.c_str());
        return;
    }
    mConditionLock.lock();
    mWaitCondition.notify_one();
    mConditionLock.unlock();

}

bool CouchDB::unPoolDocument()
{
    std::string data;
    if (!this->mpRequests->get(data)) return false;

    std::string url = "/";
    url += this->mDb;

    bool ret = HTTPCommunication::postURL(mServer,url,data,"application/json",mPort);
    if (!ret)
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
        mConditionLock.lock();
        mWaitCondition.wait(mConditionLock);
        compact = this->mMustCompact;
        this->mMustCompact = false;
        mConditionLock.unlock();
        while (unPoolDocument()); // unpool all documents until queue is empty

        if (compact)
        {
            std::string url = "/";
            url += this->mDb;
            url += "/";
            std::string st = HTTPCommunication::getURL(mServer,url,mPort);
            Dumais::JSON::JSON j;
            j.parse(st);
            Logging::log("Will compact CouchDB. Size=%s",j["disk_size"].str().c_str());

            bool ret = HTTPCommunication::postURL(mServer,url+"_compact","","application/json",mPort);
            if (!ret)
            {
                Logging::log("ERROR: Could not compact CouchDB Database");
            }
        }
    }
}

void CouchDB::createDb()
{
    std::string dbDoc = HTTPCommunication::getURL(mServer,"/"+this->mDb+"/",mPort);
    Dumais::JSON::JSON jv;
    jv.parse(dbDoc);
    if (jv["db_name"].str() == this->mDb) return;
    
    Logging::log("DB is missing in couchdb. Creating it");
    
    bool ret = HTTPCommunication::putURL(mServer,"/"+this->mDb,"","application/json",mPort);
    if (!ret)
    {
        Logging::log("ERROR: Could not add DB in CouchDB");
    }
    
}

void CouchDB::createViewsIfNonExistant(std::string views)
{
    std::string viewsDoc = HTTPCommunication::getURL(mServer,"/"+this->mDb+"/_design/views/",mPort);
    Dumais::JSON::JSON jv;
    jv.parse(viewsDoc);
    
    Dumais::JSON::JSON j;
    j.parse(views);

    Dumais::JSON::JSON& v = j["views"];
    if (!v.isValid()) return;
    
    bool oneIsMissing = false;
    for (auto& it : v.keys())
    {
        if (!jv["views"][it].isValid())
        {
            oneIsMissing = true;
            break;
        }
    }

    if (!oneIsMissing) return;

    Logging::log("Views are missing in couchdb. Creating them");
    
    if (jv["_rev"].isValid()) j.addValue(jv["_rev"].str(),"_rev");
    bool ret = HTTPCommunication::putURL(mServer,"/"+this->mDb+"/_design/views",j.stringify(false),"application/json",mPort);
    if (!ret)
    {
        Logging::log("ERROR: Could not add views in CouchDB");
    }
}
