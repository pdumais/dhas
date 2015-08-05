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
    duk_push_current_function(ctx);
    duk_get_prop_string(ctx, -1, "engine");
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
    this->context = 0;
}

void JSEngine::prepareContext()
{
    this->context = duk_create_heap_default();

    // create initiateAction function
    /*
        We push the global object on stack. Then we push the function.
        calling duk_put_prop_string will pop the previous value and store it
        in the object at index -2 (therefore, the global object). 
    */
    duk_push_global_object(context);
    duk_push_c_function(context, initiateAction, DUK_VARARGS);
    duk_push_pointer(context,this);
    duk_put_prop_string(context, -2, "engine"); //save  functionObject["engine"] = this
    duk_put_prop_string(context, -2, "initiateAction"); // globalObject["initiateAction"] = pushed function
    duk_push_c_function(context, log, DUK_VARARGS);
    duk_push_pointer(context,this);
    duk_put_prop_string(context, -2, "engine"); //save  functionObject["engine"] = this
    duk_put_prop_string(context, -2, "log"); 
    duk_pop(context);
}

JSEngine::~JSEngine()
{
    if (this->context != 0) duk_destroy_heap(this->context);
}

void JSEngine::notifyEvent(const std::string& jsonEvent)
{
    Logging::log("Script event: %s",jsonEvent.c_str());
    if (this->context == 0)
    {
        Logging::log("No script loaded");
        return;
    }
    const char *st = jsonEvent.c_str();

    duk_push_global_object(this->context);
    duk_get_prop_string(this->context, -1, "onEvent"); // get Propery "onEvent" from object at -1 (the global object)
    duk_push_string(this->context, st);
    if (duk_pcall(this->context,1) != 0)
    {
        Logging::log("ERROR: calling javascript onEvent");
    }
    duk_pop(this->context);
}

void JSEngine::load(std::string script)
{
    if (this->context != 0) duk_destroy_heap(this->context);
    this->prepareContext();
    if (duk_peval_file_noresult(this->context,script.c_str()) != 0)
    {
        Logging::log("ERROR loading script file");
        duk_destroy_heap(this->context);
        this->context = 0;
    }
}

