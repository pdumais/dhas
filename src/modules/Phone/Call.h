#ifndef CALL_H
#define CALL_H

#include <resip/dum/AppDialogSet.hxx>
#include "RTPObserver.h"
#include "RTPSession.h"
#include "json/JSON.h"
#include "IPhoneAction.h"

enum CallState
{
    Initial,
    Ringing,
    Answered
};


class Call: public resip::AppDialogSet, public Dumais::Sound::ISoundPlaylistObserver
{
public:
	Call(resip::DialogUsageManager &dum);
	~Call();

    void addRTPObserver(RTPObserver *obs);
    void removeRTPObserver(RTPObserver *obs);

    void onSoundQueueEmpty();
    void setRTPSession(Dumais::Sound::RTPSession *rtpSession);

    Dumais::Sound::RTPSession* getRTPSession();

    bool isIncomming();
    void setIncomming(bool i);
    CallState getCallState();

    void onTerminated();

    resip::DialogUsageManager& getDUM();
    std::string getDigitQueue();
    void clearDigitQueue();

    resip::NameAddr getFrom();
    resip::NameAddr getContact();
    std::string getID();
    void invokeAnswerHandler();
    void notifySoundsEmpty();

    void toJSON(Dumais::JSON::JSON& json);

private:
    // TODO: will need to unfriend this
    friend class PhoneModule;
    friend class SIPEngine;

    CallState mCallState;
    resip::InviteSessionHandle mInviteSessionHandle;

    Dumais::Sound::RTPSession *mRtpSession;
    std::list<RTPObserver*> mRtpObservers;
    bool mIncomming;
    IPhoneAction* mAnsweredAction;
    std::string mDigitQueue;
    resip::NameAddr mFrom;
    resip::NameAddr mOwner;
    resip::NameAddr mTo;
    resip::NameAddr mContact;
};

#endif

