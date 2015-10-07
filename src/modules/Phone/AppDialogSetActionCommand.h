#pragma once
#include <resip/dum/DumCommand.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/Handle.hxx>

class Call;
class IPhoneAction;
class ActionMachine;

enum class ActionCommandType
{
    Add,
    Run,
    Delete
};


class AppDialogSetActionCommand : public resip::DumCommand
{
public:
    AppDialogSetActionCommand(Call *call, IPhoneAction* action, ActionMachine *am, ActionCommandType type);
    void executeCommand();
    Message* clone() const;
    std::ostream& encode(std::ostream& strm) const;
    std::ostream& encodeBrief(std::ostream& strm) const;
private:
    Call* mpCall;
    IPhoneAction* mpAction;
    ActionMachine* mpActionMachine;
    ActionCommandType mType;
};



