#include "DHASLogging.h"
#include "PlayPhoneAction.h"
#include "Call.h"
#include "SIPEngine.h"
#include "SoundListParser.h"

PlayPhoneAction::PlayPhoneAction(SIPEngine* engine, const std::string& playString, const std::string& soundsFolder): IPhoneAction(engine)
{
    this->mPlayString = playString;
    this->mSoundsFolder = soundsFolder;
}

PlayPhoneAction::~PlayPhoneAction()
{
}

void PlayPhoneAction::invoke(Call* call)
{
    LOG("PlayPhoneAction::invoke");
    call->addRTPObserver(this);
    SoundListParser parser(this->mPlayString);
    std::vector<std::string> list = parser.getSoundList();
    if (list.size() == 0)
    {
        this->onActionTerminated(call);
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
                LOG("Queuing "<<duration<<" seconds silence on call");
                call->getRTPSession()->silence(duration);
            } else {
                std::string st = this->mSoundsFolder+(sound)+"-ulaw.wav";
                LOG("Queuing "<<st.c_str()<<" on call ");
                call->getRTPSession()->play(st.c_str(), Dumais::Sound::G711);
            }
        }
    }
}

void PlayPhoneAction::onRTPSessionSoundQueueEmpty(Call *call)
{
    LOG("RTP Sound Queue Empty");
    this->onActionTerminated(call);
}

void PlayPhoneAction::clean(Call *call)
{
    call->removeRTPObserver(this);
}
