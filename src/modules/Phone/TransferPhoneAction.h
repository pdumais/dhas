#pragma once

#include "IPhoneAction.h"
#include <string>

class TransferPhoneAction: public IPhoneAction
{
private:
    std::string mDestination;
public:
    TransferPhoneAction(SIPEngine* engine, const std::string& destination);
    virtual void invoke(Call* call);
};
