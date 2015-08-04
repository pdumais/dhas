#pragma once
#include <vector>
#include <functional>

#define REGISTER_MODULE(T) ModuleRegistrar __ ## T ## __([]()->Module*{return new T();});

class Module;

struct ModuleChainedList
{
    std::function<Module*()> create;
    ModuleChainedList* next;
};

class ModuleRegistrar
{
private:
    static ModuleChainedList* moduleChainedList;
public:

    ModuleRegistrar(std::function<Module*()> creator);
    static std::vector<Module*> createAllModules();
};

