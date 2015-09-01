#pragma once

#include <string>
#include "RESTInterface.h"
#include "IEventNotificationListener.h"

class IScriptEngine: public IEventNotificationListener{
protected:
    RESTInterface *mpRESTInterface;
public:
    void setRESTInterface(RESTInterface *pRESTInterface){mpRESTInterface=pRESTInterface;}
    RESTInterface* getRESTInterface(){return mpRESTInterface;}
    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent)=0;
    virtual void load(std::string script)=0;

};


