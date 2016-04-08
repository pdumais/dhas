#pragma once
#include <string>
#include <vector>
#include "ThreadSafeRestEngine.h"
#include "DWNSendQueue.h"

#define REGISTER_NODE_NAME(classname, name) DWNSubModuleInstantiator<classname> __ ## classname ## __(name);


class IDWN;

class IDWNSubModuleInstantiator
{
protected:
    std::string mName;    
public:
    IDWNSubModuleInstantiator()
    {
        instances.push_back(this);
    }
    std::string getName(){return mName;}
    static std::vector<IDWNSubModuleInstantiator*> instances;
    virtual IDWN* createInstance() = 0;

};

template<class T>
class DWNSubModuleInstantiator: public IDWNSubModuleInstantiator
{
public:
    DWNSubModuleInstantiator(std::string name)
    {
        mName = name;
    }

    IDWN* createInstance()
    {
        T* t = new T();
        t->setName(mName);
        return t;
    }
};


struct NodeInfo
{
    std::string id;
    std::string ip;
    DWNSendQueue* sendQueue;
};

class IDWN
{
protected: 
    std::vector<IDWNSubModuleInstantiator> mSubModulesType;
    std::vector<RESTCallBack*> mRestCallbacks;
    std::string mName;
    NodeInfo mInfo;

public:
    virtual void registerCallBacks(ThreadSafeRestEngine* pEngine) = 0;
    virtual bool processData(char* buf, size_t size, Dumais::JSON::JSON& reply) = 0;
    virtual void run(time_t t) = 0;
    virtual void addInfo(Dumais::JSON::JSON&);
    void unRegisterCallBacks(ThreadSafeRestEngine* pEngine);
    
    std::string getName() {return mName;}
    void setName(std::string name) {mName = name;}
    static std::vector<std::string> getRegisteredSubModulesNames();
    static IDWN* createDriverInstance(std::string name);

    void setIinfo(NodeInfo info){ mInfo = info; }
    NodeInfo& getInfo() {return mInfo;}
};



