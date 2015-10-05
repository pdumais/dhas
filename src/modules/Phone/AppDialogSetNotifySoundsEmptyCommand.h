#pragma once
#include <resip/dum/DumCommand.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/Handle.hxx>
#include "Call.h"

class AppDialogSetNotifySoundsEmptyCommand : public resip::DumCommand
{
public:
    AppDialogSetNotifySoundsEmptyCommand(Call *call);
    void executeCommand();
    Message* clone() const;
    std::ostream& encode(std::ostream& strm) const;
    std::ostream& encodeBrief(std::ostream& strm) const;

private:
    Call* mCall;
};


