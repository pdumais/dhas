#include "Logging.h"
#include "IPhoneAction.h"
#include "SIPEngine.h"

IPhoneAction::IPhoneAction(SIPEngine* engine)
{
    this->mpSIPEngine = engine;
    this->mpNextAction = 0;
    this->mpActionMachine = 0;
    this->mInvoked = false;
    this->mCleaned = false;
}

void IPhoneAction::setNextAction(IPhoneAction* nextAction)
{
    this->mpNextAction = nextAction;
}

void IPhoneAction::invokeAction(Call* call)
{
    if (mInvoked)
    {
        Logging::log("ERROR: This action was already invoked %i",this);
        return;
    }
    
    Logging::log("Running action %s",getName().c_str());
    mInvoked = true;
    call->setCurrentAction(this);
    this->invoke(call);
}

bool IPhoneAction::cleanAction(Call* call)
{
    if (mCleaned)
    {
        Logging::log("ERROR: This action was already cleaned %i",this);
        return false;
    }
    
    mCleaned = true;
    this->clean(call);
    return true;
}

IPhoneAction* IPhoneAction::getNextAction()
{
    return this->mpNextAction;
}

void IPhoneAction::setActionMachine(ActionMachine* am)
{
    this->mpActionMachine = am;
}

void IPhoneAction::onActionTerminated(Call* call)
{
    Logging::log("Action %s terminated",getName().c_str());
    mpActionMachine->actionTerminated(call,this);
}

