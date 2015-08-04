#include "Logging.h"
#include "Call.h"
#include <stdio.h>

#include "resip/dum/InviteSession.hxx"
#include "resip/dum/ClientInviteSession.hxx"

using namespace resip;

Call::Call(resip::DialogUsageManager &dum):AppDialogSet(dum)
{
    this->mCallState = Initial;
    this->mRtpSession = 0;
    this->mIncomming = false;
    this->mHangupAfterSounds = false;
}

Call::~Call()
{
    if (this->mRtpSession)
    {
        delete this->mRtpSession;
        this->mRtpSession = 0;
    }
}

CallState Call::getCallState()
{
    return mCallState;
}

void Call::setPlayString(std::string playString)
{
    mPlayString = playString;
}

void Call::setIncomming(bool i)
{
    this->mIncomming = i;
}

void Call::addRTPObserver(RTPObserver *obs)
{
    this->mRtpObservers.push_back(obs);
}


void Call::setRTPSession(Dumais::Sound::RTPSession *rtpSession)
{
    this->mRtpSession = rtpSession;
    this->mRtpSession->addObserver(this); //TODO should unsubscribe if overwriting observer
}

Dumais::Sound::RTPSession* Call::getRTPSession()
{
    return this->mRtpSession;
}

void Call::onSoundQueueEmpty()
{
    for (std::list<RTPObserver*>::iterator it = this->mRtpObservers.begin();it!=this->mRtpObservers.end();it++)
    {
        (*it)->onRTPSessionSoundQueueEmpty(this);
    }

}

DialogUsageManager& Call::getDUM()
{
    return this->mDum;
}

bool Call::isIncomming()
{
    return this->mIncomming;
}

std::string Call::getDigitQueue()
{
    return this->mDigitQueue;
}

void Call::clearDigitQueue()
{
    this->mDigitQueue="";
}

resip::NameAddr Call::getFrom()
{
    return mFrom;
}

resip::NameAddr Call::getContact()
{
    return mContact;
}

std::string Call::getID()
{
    return this->getDialogSetId().getCallId().c_str();
}

void Call::onTerminated()
{
}

void Call::toJSON(Dumais::JSON::JSON& json)
{
    Dumais::JSON::JSON& j = json.addObject("call");
    j.addValue(getID(),"id");
    j.addValue(mFrom.uri().getAor().data(),"from");
    j.addValue(mTo.uri().getAor().data(),"to");
}
