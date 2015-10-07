#pragma once

#include "IPhoneAction.h"
#include "RTPObserver.h"
#include <string>


// Transition to next action will be done when sound queue is empty
class PlayPhoneAction: public IPhoneAction, public RTPObserver
{
private:
    std::string mPlayString;
    std::string mSoundsFolder;

public:
    PlayPhoneAction(SIPEngine* engine, const std::string& playString, const std::string& soundsFolder);
    ~PlayPhoneAction();
    virtual void invoke(Call* call);
    virtual void clean(Call *call);
    virtual void onRTPSessionSoundQueueEmpty(Call *call);
    
    virtual std::string getName() { return "PlayPhoneAction";}

};
