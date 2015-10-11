#ifdef SCRIPTLUA

#include "DHASLogging.h"
#include "LUAEngine.h"
#include <stdio.h>
#include <sstream>
#include "json/JSON.h"

static std::map<lua_State*,LUAEngine*> LuaEngineMap;

static int initiateAction(lua_State *L)
{
    LUAEngine *pEngine = LuaEngineMap[L];
    const char* query = lua_tostring(L,1);
    //LOG("LUA: initiateAction(\"%s\")\r\n",query);
    Dumais::JSON::JSON json;
    pEngine->getRESTInterface()->processQuery(json, query);
    
    lua_pushstring(L,json.stringify(false).c_str());
    return 1;
}


static int setVar(lua_State *L)
{
    LUAEngine *pEngine = LuaEngineMap[L];
    const char* key = lua_tostring(L,1);
    const char* val = lua_tostring(L,2);

    //pEngine->getStorage()->write(key,val);

}

static int getVar(lua_State *L)
{
    LUAEngine *pEngine = LuaEngineMap[L];
    const char* key = lua_tostring(L,1);

    //TODO: how to pass value back?
}



LUAEngine::LUAEngine()
{
    mpLUA = luaL_newstate();
//    mpStorage = new PersistantStorage("storage.dat");
    luaL_openlibs(mpLUA);
    lua_register(mpLUA, "initiateAction", initiateAction);
    lua_register(mpLUA, "getVar", getVar);
    lua_register(mpLUA, "setVar", setVar);
    LuaEngineMap[mpLUA] = this;

}

LUAEngine::~LUAEngine()
{
    lua_close (mpLUA);
}

void LUAEngine::notifyEvent(const Dumais::JSON::JSON& json)
{
    std::string jsonEvent = json.stringify(false);
    LOG("Script event: %s",jsonEvent.c_str());
    const char *st = jsonEvent.c_str();
    lua_getglobal(mpLUA,"onEvent");
    lua_pushstring(mpLUA,st);
    lua_pcall(mpLUA,1,0,0);

}

void LUAEngine::load(std::string script)
{
    luaL_loadfile(mpLUA,script.c_str());
    // load main
    lua_pcall(mpLUA,0,0,0);
}

/*PersistantStorage* LUAEngine::getStorage()
{
    return mpStorage;
}*/

#endif
