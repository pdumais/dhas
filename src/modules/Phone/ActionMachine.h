#pragma once
#include <resip/dum/DialogUsageManager.hxx>
#include <thread>

class Call;
class IPhoneAction;

class ActionMachine
{
private:
    resip::DialogUsageManager* mpDum;
    std::thread::id mMainThreadId;
    bool isMainThread();
public:
    ActionMachine();

    void setDum(resip::DialogUsageManager* dum);
    void setMainThreadId(std::thread::id id);

    void asyncAddAction(Call* call,IPhoneAction* action);
    void asyncDelAction(Call* call,IPhoneAction* action);
    void asyncRunAction(Call* call);
    void destroyChain(Call* call);
    void actionTerminated(Call* call, IPhoneAction* action);
    void addAction(Call* call, IPhoneAction* action);
    void runAction(Call* call);
    void deleteAction(IPhoneAction *action);

};
