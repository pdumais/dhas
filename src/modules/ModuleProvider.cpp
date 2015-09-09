#include "Logging.h"
#include "ModuleProvider.h"

#include "InsteonModule.h"
#include "WebServer.h"
#include "PhoneModule.h"
#include "SoundModule.h"
#include "SMTPModule.h"
#include "IEventProcessor.h"
#include "RESTEngine.h"
#include "ModuleRegistrar.h"
#include <thread>

ModuleProvider::ModuleProvider(Dumais::JSON::JSON& config)
{
    auto list = ModuleRegistrar::createAllModules();
    for (auto& it:list)
    {
        it->configure(config[it->getName().c_str()]);
        this->addModule(it);
    }

}

ModuleProvider::~ModuleProvider()
{
}

void ModuleProvider::addModule(Module *p)
{
    Logging::log("Adding Module [%s]",p->getName().c_str());
    mModuleMap[p->getName()]=p;
}

Module* ModuleProvider::getModule(const std::string& name)
{
    if (mModuleMap.find(name)==mModuleMap.end()) return 0;

    return mModuleMap[name];
    
}

void ModuleProvider::disableModule(const std::string& name)
{
    auto it = mModuleMap.find(name);
    if (it == mModuleMap.end()) return;
    
    mModuleMap.erase(it); 
}

void ModuleProvider::registerCallBacks(RESTEngine* pEngine)
{
    for (ModuleMap::iterator it = mModuleMap.begin();it!=mModuleMap.end();it++)
    {
        Module *pModule = it->second;
        pModule->registerCallBacks(pEngine);
    }
}

void ModuleProvider::startModules(IEventProcessor *p)
{
    for (ModuleMap::iterator it = mModuleMap.begin();it!=mModuleMap.end();it++)
    {
        Module *pModule = it->second;
        Logging::log("Starting %s service\r\n",pModule->getName().c_str());
        pModule->startModule(p);
    }

    Logging::log("Waiting for all services to be up");
    for (ModuleMap::iterator it = mModuleMap.begin();it!=mModuleMap.end();it++)
    {
        Module *pModule = it->second;
        pModule->waitReady();
    }

    Logging::log("All services are up");

}

void ModuleProvider::stopModules()
{
    for (ModuleMap::iterator it = mModuleMap.begin();it!=mModuleMap.end();it++)
    {
        it->second->stop();
        it->second->waitCompletion();
        Logging::log("Stopped %s service\r\n",it->second->getName().c_str());
    }
}

void ModuleProvider::appendPeriodicData(Dumais::JSON::JSON& data)
{
    for (auto& it : mModuleMap)
    {
        it.second->appendPeriodicData(data);
    }
}
