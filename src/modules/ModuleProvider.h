#ifndef SERVICEPROVIDER_H
#define SERVICEPROVIDER_H
#include <map>
#include "json/JSON.h"

class RESTEngine;
class IEventProcessor;
class Module;

typedef std::map<std::string,Module*> ModuleMap;

class ModuleProvider{
private:
    ModuleMap mModuleMap;
public:
	ModuleProvider(Dumais::JSON::JSON& config);
	~ModuleProvider();

    void addModule(Module *p);
    Module* getModule(const std::string& name);

    void registerCallBacks(RESTEngine* pEngine);
    void startModules(IEventProcessor *p);
    void stopModules();
    void appendPeriodicData(Dumais::JSON::JSON& data);
    void disableModule(const std::string& name);

};

#endif

