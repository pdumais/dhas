#include "ModuleRegistrar.h"
#include "DHASLogging.h"

ModuleChainedList* ModuleRegistrar::moduleChainedList = 0;

ModuleRegistrar::ModuleRegistrar(std::function<Module*()> creator)
{
    if (moduleChainedList == 0)
    {
        moduleChainedList = new ModuleChainedList;
        moduleChainedList->create = creator;
        moduleChainedList->next = 0;
    }
    else
    {
        ModuleChainedList *m = moduleChainedList;
        while (m->next) m = m->next;
        m->next = new ModuleChainedList;
        m->next->create = creator;
        m->next->next = 0;
        
    }
}


std::vector<Module*> ModuleRegistrar::createAllModules()
{
    std::vector<Module*> list;
    ModuleChainedList *m = moduleChainedList;
    while (m)
    {
        list.push_back(m->create());
        m = m->next;
    }
    return list;
}

