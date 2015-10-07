#pragma once

#include "IPhoneAction.h"
#include <string>

// This action does not transtion to another one since the call gets terminated
class TransferPhoneAction: public IPhoneAction
{
private:
    std::string mDestination;
public:
    TransferPhoneAction(SIPEngine* engine, const std::string& destination);
    virtual void invoke(Call* call);
    virtual void clean(Call* call) {};
    virtual std::string getName() { return "TransferPhoneAction";}
};
