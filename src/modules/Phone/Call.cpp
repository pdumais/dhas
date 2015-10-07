#include "Logging.h"
#include "Call.h"
#include <stdio.h>

#include "resip/dum/InviteSession.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "AppDialogSetNotifySoundsEmptyCommand.h"

using namespace resip;

Call::Call(resip::DialogUsageManager &dum, ActionMachine* am):AppDialogSet(dum)
{
    this->mpActionMachine = am;
    this->mCallState = Initial;
    this->mRtpSession = 0;
    this->mIncomming = false;
    this->mpCurrentAction = 0;
}

Call::~Call()
{
    mpActionMachine->destroyChain(this);

    if (this->mRtpSession)
    {
        delete this->mRtpSession;
        this->mRtpSession = 0;
    }
}

void Call::setCurrentAction(IPhoneAction* a)
{
    this->mpCurrentAction = a;
}

void Call::onCallAnswered()
{
    // The call is answered by UAS, this is what triggers the action chain
    mpActionMachine->asyncRunAction(this);
}

CallState Call::getCallState()
{
    return mCallState;
}

void Call::setIncomming(bool i)
{
    this->mIncomming = i;
}

IPhoneAction* Call::getCurrentAction()
{
    return mpCurrentAction;
}

void Call::removeRTPObserver(RTPObserver *obs)
{
    for (auto it = this->mRtpObservers.begin(); it != this->mRtpObservers.end(); it++)
    {
        if (*it != obs) continue;
        this->mRtpObservers.erase(it);
        return; 
    }
}

void Call::addRTPObserver(RTPObserver *obs)
{
    for (auto it = this->mRtpObservers.begin(); it != this->mRtpObservers.end(); it++)
    {
        if (*it == obs) return;
    }

    this->mRtpObservers.push_back(obs);
}

void Call::appendAction(IPhoneAction* action)
{
    if (mpCurrentAction == 0)
    {
        mpCurrentAction = action;

        //if no action was set and the call was answered, then schedule the action to run
        if (getCallState() == Answered) mpActionMachine->asyncRunAction(this);
    }
    else
    {
        // if an action is already in queue, it means that we can just append to it and
        // it will eventually get invoked
        IPhoneAction *a = mpCurrentAction;
        while (a->getNextAction() != 0)
        {
            a = a->getNextAction();    
        }
        a->setNextAction(action);
    }
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

//WARNING: This is called from the sound player thread
void Call::onSoundQueueEmpty()
{
    // Don't notify from this thread, post it on the dum's thread
    AppDialogSetNotifySoundsEmptyCommand* cmd = new AppDialogSetNotifySoundsEmptyCommand(this);
    this->mDum.post(cmd);
}

void Call::notifySoundsEmpty()
{
    // we copy the list because observers could unsubscribe in the meantime
    auto list = this->mRtpObservers;
    for (std::list<RTPObserver*>::iterator it = list.begin();it!=list.end();it++)
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
