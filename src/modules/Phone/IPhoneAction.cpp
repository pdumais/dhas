#include "DHASLogging.h"
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
        LOG("ERROR: This action was already invoked "<<this);
        return;
    }
    
    LOG("Running action "<<getName().c_str());
    mInvoked = true;
    call->setCurrentAction(this);
    this->invoke(call);
}

bool IPhoneAction::cleanAction(Call* call)
{
    if (mCleaned)
    {
        LOG("ERROR: This action was already cleaned "<<this);
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
    LOG("Action "<< getName().c_str() <<" terminated");
    mpActionMachine->actionTerminated(call,this);
}

