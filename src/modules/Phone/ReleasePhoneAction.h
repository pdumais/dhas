#pragma once

#include "IPhoneAction.h"

// This action does not transtion to another one since the call gets terminated
class ReleasePhoneAction: public IPhoneAction
{
public:
    ReleasePhoneAction(SIPEngine* engine);
    virtual void invoke(Call* call);
    virtual void clean(Call *c) {};
    virtual std::string getName() { return "ReleasePhoneAction";}
};
