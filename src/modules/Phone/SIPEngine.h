#ifndef SIPENGINE_H
#define SIPENGINE_H
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "RegistrationManager.h"
#include "AccountSettings.h"
#include "RegistrationObserver.h"
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/stack/SipStack.hxx>
#include <resip/dum/MasterProfile.hxx>
#include <resip/dum/SubscriptionHandler.hxx>
#include <resip/dum/OutOfDialogHandler.hxx>
#include "Call.h"
#include "AppDialogSetEndCommand.h"
#include "SoundDeviceFactory.h"
#include "TelephonyObserver.h"
#include <thread>
#include "ActionMachine.h"

struct DevicePresence
{
    std::string device;
    std::string status;
    resip::ClientSubscriptionHandle subscription;
};


class SIPEngine: public RegistrationObserver, public resip::InviteSessionHandler, public resip::OutOfDialogHandler, public resip::ClientSubscriptionHandler
{
private:
    TelephonyObserver *mpObserver;
    AccountSettings settings;
    RegistrationManager mRegistrationManager;
    resip::SipStack *mSipStack;
    resip::DialogUsageManager* mDum;
    resip::SharedPtr<resip::MasterProfile> mProfile;
    bool mIsRegistered;
    int mRONATimeout;
    Dumais::Sound::SoundDeviceFactory *mpSoundDeviceFactory;
    bool mStopping;
    ActionMachine *mpActionMachine;
public:
	SIPEngine(int rtplow, int rtphigh, std::string localip, int sipport, int ronatimeout, ActionMachine *am);
	~SIPEngine();

    Call* makeCall(std::string extension);
    void transferCall(Call* call, const std::string& extension);
    void start();
    void stop();
    void run();
    void setTelephonyObserver(TelephonyObserver *pObserver);
    void registerUserAgent(AccountSettings accountSettings);
    void releaseCall(Call *call);
    void subscribeBLF(std::string device);
    void subscribeMWI(std::string device);

    virtual void onRegistered(bool registered);
    
    virtual void onSuccess(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& successResponse);
    virtual void onFailure(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& errorResponse);
    virtual void onReceivedRequest(resip::ServerOutOfDialogReqHandle, const resip::SipMessage& request);

    virtual void onUpdatePending (resip::ClientSubscriptionHandle, const resip::SipMessage &notify, bool outOfOrder);
    virtual void onUpdateActive (resip::ClientSubscriptionHandle, const resip::SipMessage &notify, bool outOfOrder);
    virtual void onUpdateExtension (resip::ClientSubscriptionHandle, const resip::SipMessage &notify, bool outOfOrder);
    virtual int onRequestRetry (resip::ClientSubscriptionHandle, int retrySeconds, const resip::SipMessage &notify);
    virtual void onTerminated (resip::ClientSubscriptionHandle, const resip::SipMessage *msg);
    virtual void onNewSubscription (resip::ClientSubscriptionHandle, const resip::SipMessage &notify);


    virtual void onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
    virtual void onFailure(resip::ClientRegistrationHandle, const resip::SipMessage& msg);
    virtual void onMessage(resip::Handle<resip::InviteSession>, const resip::SipMessage& msg);
    virtual void onMessageSuccess(resip::Handle<resip::InviteSession>, const resip::SipMessage&);
    virtual void onMessageFailure(resip::Handle<resip::InviteSession>, const resip::SipMessage&);
    virtual void onFailure(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg);
    virtual void onForkDestroyed(resip::ClientInviteSessionHandle);
    virtual void onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onProvisional(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg);
    virtual void onConnected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
    virtual void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onStaleCallTimeout(resip::ClientInviteSessionHandle);
    virtual void onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
    virtual void onAnswer(resip::InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& sdp);
    virtual void onEarlyMedia(resip::ClientInviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents& sdp);
    virtual void onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onOfferRejected(resip::Handle<resip::InviteSession>, const resip::SipMessage *msg);
    virtual void onInfo(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onRefer(resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg);
    virtual void onReferAccepted(resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg);
    virtual void onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onReferNoSub(resip::Handle<resip::InviteSession>, const resip::SipMessage&);
    virtual void onRemoved(resip::ClientRegistrationHandle);
    virtual int onRequestRetry(resip::ClientRegistrationHandle, int retrySeconds, const resip::SipMessage& response);
    virtual void onNewSession(resip::ServerInviteSessionHandle sis, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
    virtual void onNewSession(resip::ClientInviteSessionHandle cis, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
    virtual void onTerminated(resip::InviteSessionHandle is, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg);
    virtual void onOffer(resip::InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& sdp);
    
    void answerIncomming(Call* call);
    
};

#endif

