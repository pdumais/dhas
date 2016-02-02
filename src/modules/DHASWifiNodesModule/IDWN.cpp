#include "IDWN.h"

std::vector<IDWNSubModuleInstantiator*> IDWNSubModuleInstantiator::instances;

std::vector<std::string> IDWN::getRegisteredSubModulesNames()
{
    std::vector<std::string> list;
    for (auto& it : IDWNSubModuleInstantiator::instances)
    {
        list.push_back(it->getName());
    }
    return list;
}

IDWN* IDWN::createDriverInstance(std::string name)
{
    for (auto& it : IDWNSubModuleInstantiator::instances)
    {
        if (it->getName() == name)
        {
            return it->createInstance();
        }
    }
    return 0;
}

void IDWN::unRegisterCallBacks(ThreadSafeRestEngine* pEngine)
{
    for (auto& it : mRestCallbacks)
    {
        pEngine->removeCallBack(it);
    }
}
