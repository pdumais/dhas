#include "Logging.h"
#include "CallFactory.h"
#include "Call.h"

using namespace resip;

CallFactory::CallFactory(){
}

CallFactory::~CallFactory(){
}

AppDialogSet* CallFactory::createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg)
{
    Call *call = new Call(dum);
    return call;
}

