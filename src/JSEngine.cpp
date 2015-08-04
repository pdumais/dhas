#include "Logging.h"
#include "JSEngine.h"
#include <stdio.h>
#include <sstream>
#include "JSON.h"

int log(duk_context *ctx)
{
    const char* query = duk_to_string(ctx, 0);
    Logging::log("jscript: %s",query);
    return 0;
}

int initiateAction(duk_context *ctx)
{
    // retrieve instance of JSEngine object from stash
    duk_push_heap_stash(ctx);
    duk_push_pointer(ctx,ctx);
    duk_get_prop(ctx,-2);
    JSEngine* pEngine = duk_get_pointer(ctx,-1);
    duk_pop_n(ctx,2);

    // Get the query and execute it
    const char* query = duk_to_string(ctx, 0);
    Dumais::JSON::JSON json;
    if (pEngine->getRESTInterface())
    {
        pEngine->getRESTInterface()->processQuery(json, query);
    }
    else
    {
        json.addValue("error","result");
    }    
    duk_push_string(ctx,json.stringify(false).c_str());
    return 1;
}


JSEngine::JSEngine()
{
    this->context = duk_create_heap_default();

    // create initiateAction function
    duk_push_global_object(context);
    duk_push_c_function(context, initiateAction, DUK_VARARGS);
    duk_put_prop_string(context, -2, "initiateAction");
    duk_pop(context);

    // Create log function
    duk_push_global_object(context);
    duk_push_c_function(context, log, DUK_VARARGS);
    duk_put_prop_string(context, -2, "log");
    duk_pop(context);

    // save instance of this object in stash
    duk_push_heap_stash(this->context);
    duk_push_pointer(this->context,this->context);
    duk_push_pointer(this->context,this);
    duk_put_prop(this->context,-3);
    duk_pop(this->context);
}

JSEngine::~JSEngine()
{
    duk_destroy_heap(this->context);
}

void JSEngine::notifyEvent(const std::string& jsonEvent)
{
    Logging::log("Script event: %s",jsonEvent.c_str());
    const char *st = jsonEvent.c_str();
//    lua_getglobal(mpLUA,"onEvent");
//    lua_pushstring(mpLUA,st);
//    lua_pcall(mpLUA,1,0,0);

}

void JSEngine::load(std::string script)
{
    duk_eval_file_noresult(this->context,script.c_str());
}


