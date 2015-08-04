#include "Logging.h"
#include "AppDialogSetEndCommand.h"  
#include <stdio.h> 
#include <thread>

using namespace resip;
AppDialogSetEndCommand::AppDialogSetEndCommand(AppDialogSetHandle h) : mHandle(h)
{
}

void AppDialogSetEndCommand::executeCommand()
{ 
    if(mHandle.isValid()) 
    { 
        Logging::log("%i AppDialogSetEndCommand::executeCommand\r\n", std::this_thread::get_id());
        mHandle->end();
    }
}

Message* AppDialogSetEndCommand::clone() const 
{
    assert(false); 
    return 0; 
}
std::ostream& AppDialogSetEndCommand::encode(std::ostream& strm) const 
{
    return strm; 
}
std::ostream& AppDialogSetEndCommand::encodeBrief(std::ostream& strm) const 
{
    return strm; 
}
