#include "Logging.h"
#include "PlayPhoneAction.h"
#include "Call.h"
#include "SIPEngine.h"
#include "SoundListParser.h"

PlayPhoneAction::PlayPhoneAction(SIPEngine* engine, const std::string& playString, const std::string& soundsFolder): IPhoneAction(engine)
{
    this->mSoundListEmptyAction = 0;
    this->mPlayString = playString;
    this->mSoundsFolder = soundsFolder;
}

PlayPhoneAction::~PlayPhoneAction()
{
    if (this->mSoundListEmptyAction)
    {
        delete this->mSoundListEmptyAction;
    }

    this->mSoundListEmptyAction = 0;

    // Don't need to remove ourself from RTP observer list of call since the call's destructor
    // is deleting this object.
}

void PlayPhoneAction::invoke(Call* call)
{
    call->addRTPObserver(this);
    SoundListParser parser(this->mPlayString);
    std::vector<std::string> list = parser.getSoundList();
    if (list.size() == 0)
    {
        if (this->mSoundListEmptyAction) this->mSoundListEmptyAction->invoke(call);
    }
    else
    {
        for (std::vector<std::string>::iterator it = list.begin();it!=list.end();it++)
        {
            std::string sound = *it;
            if (sound[0]=='$')
            {
                std::string st = sound.substr(1,std::string::npos);
                int duration = atoi(st.c_str());
                Logging::log("Queuing %i seconds silence on call \r\n",duration);
                call->getRTPSession()->silence(duration);
            } else {
                std::string st = this->mSoundsFolder+(sound)+"-ulaw.wav";
                Logging::log("Queuing %s on call \r\n",st.c_str());
                call->getRTPSession()->play(st.c_str(), Dumais::Sound::G711);
            }
        }
    }
}

void PlayPhoneAction::setSoundsEmptyAction(IPhoneAction* a)
{
    this->mSoundListEmptyAction = a;
}

void PlayPhoneAction::onRTPSessionSoundQueueEmpty(Call *call)
{
    Logging::log("RTP Sound Queue Empty");
    if (this->mSoundListEmptyAction) this->mSoundListEmptyAction->invoke(call);
    call->removeRTPObserver(this);
}
