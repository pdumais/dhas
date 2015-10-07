#pragma once
#include <string>
class SIPEngine;
class Call;
class ActionMachine;

/*
    When an action terminates, it will call this base class onActionTerminated.
    That method will signal the ActionMachine. The ActionMachine will take care
    of cleaning the current action, schedule its deletion and set the next action to run

    NOTE: Some actions don't call onActionTerminated because they are actions that terminates the call
    Those who call onActionTerminated do so when their action completes. This can be asynchronous.
    For example: The PlaySound action terminates when the sound queue is empty.
*/

class IPhoneAction
{
private:
    ActionMachine *mpActionMachine;
    bool mInvoked;
    bool mCleaned;

protected:
    IPhoneAction* mpNextAction;
    SIPEngine *mpSIPEngine;
    

    void onActionTerminated(Call* call);
    virtual void invoke(Call* call)=0;
    virtual void clean(Call *call)=0;

public:
    IPhoneAction(SIPEngine* engine);
    ~IPhoneAction(){};

    void setNextAction(IPhoneAction* nextAction);
    IPhoneAction* getNextAction();
    void setActionMachine(ActionMachine* am);

    void invokeAction(Call* call);
    bool cleanAction(Call* call);

    virtual std::string getName() = 0;
};
