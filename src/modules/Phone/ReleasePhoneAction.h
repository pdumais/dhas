#pragma once

#include "IPhoneAction.h"

class ReleasePhoneAction: public IPhoneAction
{
public:
    ReleasePhoneAction(SIPEngine* engine);
    virtual void invoke(Call* call);
};
