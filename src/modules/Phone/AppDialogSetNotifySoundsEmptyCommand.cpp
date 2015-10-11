#include "DHASLogging.h"
#include "AppDialogSetNotifySoundsEmptyCommand.h"  
#include <stdio.h> 
#include <thread>

using namespace resip;
AppDialogSetNotifySoundsEmptyCommand::AppDialogSetNotifySoundsEmptyCommand(Call *c) : mCall(c)
{
}

void AppDialogSetNotifySoundsEmptyCommand::executeCommand()
{ 
    mCall->notifySoundsEmpty();
}

Message* AppDialogSetNotifySoundsEmptyCommand::clone() const 
{
    assert(false); 
    return 0; 
}
std::ostream& AppDialogSetNotifySoundsEmptyCommand::encode(std::ostream& strm) const 
{
    return strm; 
}
std::ostream& AppDialogSetNotifySoundsEmptyCommand::encodeBrief(std::ostream& strm) const 
{
    return strm; 
}
