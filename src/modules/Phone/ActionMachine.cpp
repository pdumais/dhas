#include "Logging.h"
#include "ActionMachine.h"
#include "Call.h"
#include "IPhoneAction.h"
#include "AppDialogSetActionCommand.h"

ActionMachine::ActionMachine()
{
    this->mpDum = 0;
}

void ActionMachine::setDum(resip::DialogUsageManager* dum)
{
    this->mpDum = dum;
}

void ActionMachine::asyncAddAction(Call* call,IPhoneAction* action)
{
    //Always post even if same thread to avoid callback hell
    AppDialogSetActionCommand* cmd = new AppDialogSetActionCommand(call,action,this,ActionCommandType::Add);
    this->mpDum->post(cmd);
}

void ActionMachine::asyncDelAction(Call* call,IPhoneAction* action)
{
    Logging::log("Scheduling deletion of action %s",action->getName().c_str());

    // We do the claning here because we want to be sure that starting immediately, the action
    // wont get any callbacks. Also, the owning call could be deleted by the time we delete the action.
    if (!action->cleanAction(call)) return;

    //Always post even if same thread to avoid callback hell
    AppDialogSetActionCommand* cmd = new AppDialogSetActionCommand(call,action,this,ActionCommandType::Delete);
    this->mpDum->post(cmd);
}

void ActionMachine::asyncRunAction(Call* call)
{
    //Always post even if same thread to avoid callback hell
    AppDialogSetActionCommand* cmd = new AppDialogSetActionCommand(call,(IPhoneAction*)0,this,ActionCommandType::Run);
    this->mpDum->post(cmd);
}

void ActionMachine::destroyChain(Call* call)
{
    if (!isMainThread()) Logging::log("ERROR: ActionMachine::destroyChain called from another thread");
    IPhoneAction* action = call->getCurrentAction();
    if (!action) return;

    call->setCurrentAction(0);
    while (action)
    {
        this->asyncDelAction(call, action);
        action = action->getNextAction();
    }
}

void ActionMachine::actionTerminated(Call* call, IPhoneAction* action)
{
    if (!isMainThread()) Logging::log("ERROR: ActionMachine::actionTerminated called from another thread");

    IPhoneAction *nextAction = action->getNextAction();
    call->setCurrentAction(nextAction);
    asyncDelAction(call, action);
    if (nextAction) asyncRunAction(call);
}
    
void ActionMachine::addAction(Call* call, IPhoneAction* action)
{
    if (!isMainThread()) Logging::log("ERROR: ActionMachine::addAction called from another thread");

    action->setActionMachine(this);
    call->appendAction(action);
}
    
void ActionMachine::runAction(Call* call)
{
    if (!isMainThread()) Logging::log("ERROR: ActionMachine::runAction called from another thread");

    IPhoneAction *action = call->getCurrentAction();
    if (!action) return;

    // invokeAction is smart enough to know not to be executed twice
    action->invokeAction(call);
}
    
void ActionMachine::deleteAction(IPhoneAction *action)
{
    if (!isMainThread()) Logging::log("ERROR: ActionMachine::deleteAction called from another thread");

    Logging::log("Deleting action %s",action->getName().c_str());

    // It would be nice to make sure that call.currentAction is not set to that action
    // but at this point, the call could already have been deleted
    delete action;
}
    
void ActionMachine::setMainThreadId(std::thread::id id)
{
    mMainThreadId = id;
}

bool ActionMachine::isMainThread()
{
    return (std::this_thread::get_id() == mMainThreadId);
}

