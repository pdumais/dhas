#include "DHASLogging.h"
#include "AppDialogSetActionCommand.h"  
#include <stdio.h> 
#include <thread>
#include "ActionMachine.h"

using namespace resip;
AppDialogSetActionCommand::AppDialogSetActionCommand(Call *call, IPhoneAction* action, ActionMachine *am, ActionCommandType type)
{
    mpCall = call;
    mpAction = action;
    mpActionMachine = am;
    mType = type;
}

void AppDialogSetActionCommand::executeCommand()
{ 
    LOG("AppDialogSetActionCommand::executeCommand " << (int)mType);
    switch (mType)
    {
        case ActionCommandType::Add:
        {
            mpActionMachine->addAction(mpCall,mpAction);
        }
        break;
        case ActionCommandType::Run:
        {
            mpActionMachine->runAction(mpCall);
        }
        break;
        case ActionCommandType::Delete:
        {
            mpActionMachine->deleteAction(mpAction);
        }
        break;
    }

}

Message* AppDialogSetActionCommand::clone() const 
{
    assert(false); 
    return 0; 
}
std::ostream& AppDialogSetActionCommand::encode(std::ostream& strm) const 
{
    return strm; 
}
std::ostream& AppDialogSetActionCommand::encodeBrief(std::ostream& strm) const 
{
    return strm; 
}
