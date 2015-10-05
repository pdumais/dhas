#pragma once

#include "IPhoneAction.h"
#include "RTPObserver.h"
#include <string>

class PlayPhoneAction: public IPhoneAction, public RTPObserver
{
private:
    IPhoneAction* mSoundListEmptyAction;
    std::string mPlayString;
    std::string mSoundsFolder;

public:
    PlayPhoneAction(SIPEngine* engine, const std::string& playString, const std::string& soundsFolder);
    ~PlayPhoneAction();
    virtual void invoke(Call* call);
    virtual void onRTPSessionSoundQueueEmpty(Call *call);

    void setSoundsEmptyAction(IPhoneAction* a);
};
