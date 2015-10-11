#include "DHASLogging.h"
#include "CallFactory.h"
#include "Call.h"

using namespace resip;

CallFactory::CallFactory(ActionMachine *am)
{
    this->mpActionMachine = am;
}

CallFactory::~CallFactory()
{
}

AppDialogSet* CallFactory::createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg)
{
    Call *call = new Call(dum,this->mpActionMachine);
    return call;
}

