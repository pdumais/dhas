#pragma once

#ifdef SCRIPTJS

#include "IScriptEngine.h"
#include "duktape.h"


class JSEngine: public IScriptEngine
{
private:
    duk_context* context;
    void prepareContext();

public:
	JSEngine();
	~JSEngine();

    virtual void notifyEvent(const std::string& jsonEvent);
    virtual void load(std::string script); 

};

#endif
