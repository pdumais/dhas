#ifdef SCRIPTLUA

#ifndef LUAENGINE_H
#define LUAENGINE_H

#include "IScriptEngine.h"
//#include "PersistantStorage.h"
#include <map>

#include <lua.hpp>


class LUAEngine: public IScriptEngine
{
private:
    lua_State *mpLUA;
//    PersistantStorage *mpStorage;
public:
	LUAEngine();
	~LUAEngine();

    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent);
    virtual void load(std::string script); 

 //   PersistantStorage *getStorage();

};

#endif
#endif
