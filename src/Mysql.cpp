#include "Mysql.h"
#include "DHASLogging.h"

#define MAXREQUESTS 100

using namespace Dumais::Utils;

Mysql::Mysql(const std::string& db, const std::string& server, int port, const std::string& user, const std::string& password)
{
    mServer = server;
    mPort = port;
    this->mpRequests = new MPSCRingBuffer<std::string>(MAXREQUESTS);
    this->mDb = db;
    mUser = user;
    mPassword = password;

    createDb();

    mpMysql = new MYSQL();
    mysql_init(mpMysql);
    mysql_real_connect(mpMysql,mServer.c_str(),user.c_str(),password.c_str(),mDb.c_str(),0,0,0);    

    mThreadHandle = std::thread([this](){
        this->run();
    });
}

Mysql::~Mysql()
{
    this->mStopping = true;
    
    mConditionLock.lock();
    mWaitCondition.notify_one();
    mConditionLock.unlock();

    if (mThreadHandle.joinable()) mThreadHandle.join();
    delete mpMysql;
}

void Mysql::addDocument(const Dumais::JSON::JSON& json)
{
    std::string st = json.stringify(false);
    if (!this->mpRequests->put(st))
    {
        LOG("Mysql event queue overlofw. Dropping " << st.c_str());
        return;
    }
    mConditionLock.lock();
    mWaitCondition.notify_one();
    mConditionLock.unlock();

}

bool Mysql::unPoolDocument()
{
    std::string data;
    if (!this->mpRequests->get(data)) return false;

    Dumais::JSON::JSON j;
    j.parse(data);

    char str[2048];
    int len = 0;
    len = mysql_real_escape_string(mpMysql,str, data.c_str(),data.size());

    std::stringstream ss;
    ss << "insert into events (timestamp,event,data) VALUES(";
    ss << j["timestamp"].toInt();
    ss << ",'";
    ss << j["event"].str();
    ss << "','";
    ss << str;
    ss << "');";
    std::string cmd = ss.str();
    int err = mysql_real_query(mpMysql,cmd.c_str(),cmd.size());
    if (err)
    {
        LOG("ERROR: Could not add document in Mysql: " << err << " (" << mysql_error(mpMysql) <<")");
        if (err == 1)
        {
            delete mpMysql;
            mpMysql = new MYSQL();
            mysql_init(mpMysql);
            mysql_real_connect(mpMysql,mServer.c_str(),mUser.c_str(),mPassword.c_str(),mDb.c_str(),0,0,0);    
        }
    }
    return true;
}

void Mysql::run()
{
    this->mStopping = false;
    while (!this->mStopping)
    {
        bool compact = false;
        mConditionLock.lock();
        mWaitCondition.wait(mConditionLock);
        mConditionLock.unlock();
        while (unPoolDocument()); // unpool all documents until queue is empty
    }
}

void Mysql::createDb()
{
    MYSQL *m = new MYSQL();
    mysql_init(m);
    mysql_real_connect(m,mServer.c_str(),mUser.c_str(),mPassword.c_str(),0,0,0,0);    
    mysql_query(m,"create database dhas"); 
    mysql_query(m,"use dhas"); 
    mysql_query(m,"create table events(id int not null auto_increment, timestamp int, event varchar(255), data blob, primary key(id));");
    mysql_close(m);
}

