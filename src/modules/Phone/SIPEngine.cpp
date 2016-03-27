#include "SIPEngine.h"
#include "DHASLogging.h"
#include <resip/dum/ClientAuthManager.hxx>
#include <resip/stack/MessageWaitingContents.hxx>
#include <rutil/Log.hxx>
#include <sstream>
#include "CallFactory.h"
#include "Subscription.h"
#include <resip/stack/SdpContents.hxx>
#include <resip/dum/ServerOutOfDialogReq.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include "SoundListParser.h"
#include "AppDialogSetRONACommand.h"
#include <resip/stack/SipMessage.hxx>
#include <resip/stack/Pidf.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <rutil/XMLCursor.hxx>
#include <thread>

using namespace resip;


SIPEngine::SIPEngine(int rtplow, int rtphigh, std::string localip, int sipport, int ronatimeout, ActionMachine *actionMachine): mProfile(new MasterProfile){

//    Log::setLevel(Log::Debug);
    Log::setLevel(Log::None);

    this->mRONATimeout = ronatimeout;

    mpSoundDeviceFactory = new Dumais::Sound::SoundDeviceFactory(rtplow,rtphigh);
    mpSoundDeviceFactory->setLocalIP(localip);
    // Create SipStack
    this->mSipStack = new SipStack();
    // Add Transport
    this->mSipStack->addTransport(UDP,sipport);
    // Create DUM
    this->mDum = new DialogUsageManager(*this->mSipStack);
    actionMachine->setDum(this->mDum);

    this->mRegistrationManager.setDum(this->mDum);
    this->mRegistrationManager.addObserver(this);

    this->mpActionMachine = actionMachine;

    //Set Handlers
    this->mDum->setClientRegistrationHandler(&this->mRegistrationManager);
    std::auto_ptr<AppDialogSetFactory> factory(new CallFactory(actionMachine));
    this->mDum->setAppDialogSetFactory(factory);

    this->mProfile->setUserAgent("DumaisHomeAutomation");
    this->mProfile->addSupportedMethod(NOTIFY);
    this->mProfile->addSupportedMimeType(resip::NOTIFY, resip::Mime("application", "pidf+xml"));
    this->mProfile->addSupportedMimeType(resip::NOTIFY, resip::Mime("application", "dialog-info+xml"));
    this->mProfile->addSupportedMimeType(resip::NOTIFY, resip::Mime("application", "simple-message-summary"));
    this->mProfile->addSupportedMethod(resip::INFO);
    this->mProfile->addSupportedMimeType(resip::INFO, resip::Mime("application", "dtmf-relay"));
    this->mDum->setMasterProfile(this->mProfile);
    this->mDum->setInviteSessionHandler(this);
    this->mDum->addClientSubscriptionHandler("presence",this);
    this->mDum->addClientSubscriptionHandler("message-summary",this);
    this->mDum->addOutOfDialogHandler(NOTIFY,this);
    std::auto_ptr<ClientAuthManager> clientAuth(new ClientAuthManager);
    this->mDum->setClientAuthManager(clientAuth);

    mIsRegistered = false;
}

SIPEngine::~SIPEngine()
{
    delete mpSoundDeviceFactory;
}

void SIPEngine::start()
{
}

void SIPEngine::stop()
{
}

// Unfortunately with asterisk, this will only subscribe to the sum of messages 
// in all mailboxes that belongs to the user agent. So we cannot get MWI for 2 different mailboxes
void SIPEngine::subscribeMWI(std::string device)
{
    std::stringstream sst;
    sst << "sip:";
    sst << device << "@" <<settings.mProxy;
    NameAddr to(sst.str().c_str());

    SharedPtr<SipMessage> subMessage = mDum->makeSubscription(to, "message-summary");
    subMessage->header(h_Accepts).push_back(resip::Mime("application","simple-message-summary"));
    subMessage->header(h_Expires).value()=3600;
    mDum->send(subMessage);
}


void SIPEngine::subscribeBLF(std::string device)
{
    std::stringstream sst;
    sst << "sip:";
    sst << device << "@" <<settings.mProxy;
    NameAddr to(sst.str().c_str());

    Subscription* pSubscription = new Subscription(*mDum);
    pSubscription->mDevice = device;
    pSubscription->mDeviceStatus = "N/A";
    
    // Unlike what I initially thought, the subscription needs to be made to "dialog" and not "presence"
    SharedPtr<SipMessage> subMessage = mDum->makeSubscription(to, "presence",pSubscription);
    subMessage->header(h_Accepts).push_back(resip::Mime("application","pidf+xml"));
//    SharedPtr<SipMessage> subMessage = mDum->makeSubscription(to, "dialog",pSubscription);
//    subMessage->header(h_Accepts).push_back(resip::Mime("application","dialog-info+xml"));
    subMessage->header(h_Expires).value()=60;
    mDum->send(subMessage);
}

void SIPEngine::releaseCall(Call *pCall)
{
    LOG("Will post End Command for "<<pCall->getID().c_str());
    AppDialogSetEndCommand* cmd = new AppDialogSetEndCommand(pCall->getHandle());
    this->mDum->post(cmd);
}

void SIPEngine::transferCall(Call* call, const std::string& ext)
{
    LOG("Blind transferring to "<<ext.c_str());

//    SharedPtr<UserProfile> up(mDum->getMasterUserProfile());


    std::stringstream sst;
    sst << "sip:";
    sst << ext << "@" <<settings.mProxy;
    NameAddr to(sst.str().c_str());

    /*  refer with implicit subscription. If we specify "no implicit subscription" (rfc 4488),
        then the recipient will not send NOTIFYs to us.
    */
    call->mInviteSessionHandle.get()->refer(to);
    releaseCall(call); //TODO: should wait for notify before killing the call
}

Call* SIPEngine::makeCall(std::string extension)
{
    SharedPtr<UserProfile> up(mDum->getMasterUserProfile());
    if (!this->mIsRegistered) return 0;
    NameAddr dest;
    std::string uri = "sip:" + extension + "@" + settings.mProxy;
    dest = NameAddr(Uri(uri.c_str()));
    LOG("Attempting to call "<<uri.c_str());

    Call *call = new Call(*mDum, mpActionMachine);
    Dumais::Sound::RTPSession *rtpSession = this->mpSoundDeviceFactory->createRTPSession();
    call->setRTPSession(rtpSession);
    std::stringstream ss;
    ss<<"v=0\r\n";
    ss<<"o=- 0 0 IN IP4 "<<rtpSession->getLocalIP()<<"\r\n";
    ss<<"s="<<settings.mUserName<<"\r\n";
    ss<<"c=IN IP4 "<<rtpSession->getLocalIP()<<"\r\n";
    ss<<"t=0 0\r\n";
    ss<<"m=audio "<<rtpSession->getLocalPort()<<" RTP/AVP 0 \r\n"; // only support G.711 uLaw
    ss<<"a=rtpmap:0 pcmu/8000\r\n";

    Data txt(ss.str().c_str());
    HeaderFieldValue hfv(txt.data(), txt.size());
    Mime type("application", "sdp");
    SdpContents sdp(hfv, type);
    UInt64 currentTime = Timer::getTimeMicroSec();
    sdp.session().origin().getSessionId() = currentTime;
    sdp.session().origin().getVersion() = currentTime;

    SharedPtr<SipMessage> msg = mDum->makeInviteSession(dest, up, &sdp, call);
    call->mFrom = msg->header(h_From);
    call->mTo = dest;
    mDum->send(msg);

    return call;
}


void SIPEngine::run()
{
    FdSet fdset;
    this->mSipStack->buildFdSet(fdset);
    int err = fdset.selectMilliSeconds(resipMin((int)mSipStack->getTimeTillNextProcessMS(), 200));
    this->mSipStack->process(fdset);
    while(this->mDum->process());
}

void SIPEngine::registerUserAgent(AccountSettings accountSettings)
{
    settings = accountSettings;
    std::stringstream sst;
    sst << "sip:";
    sst << settings.mUserName << "@" <<settings.mProxy;
    NameAddr from(sst.str().c_str());

    from.displayName()=settings.mDisplayName.c_str();
    Uri outboundProxy;
    outboundProxy.host() = this->settings.mProxy.c_str();
    this->mProfile->setDigestCredential(settings.mProxy.c_str(),settings.mUserName.c_str(),settings.mPin.c_str());
    this->mProfile->setDefaultFrom(from);

    mRegistrationManager.registerAccount(settings);
}


void SIPEngine::onRegistered(bool registered)
{
    if (registered)
    {
        mIsRegistered = true;
        LOG("User agent is registered");
    } else {
        LOG("User agent failed to register");
    }
}

void SIPEngine::setTelephonyObserver(TelephonyObserver *pObserver)
{
    mpObserver = pObserver;
}

void SIPEngine::onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
{
}

void SIPEngine::onFailure(resip::ClientRegistrationHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onMessage(resip::Handle<resip::InviteSession>, const resip::SipMessage& msg)
{
}

void SIPEngine::onMessageSuccess(resip::Handle<resip::InviteSession>, const resip::SipMessage&)
{
}

void SIPEngine::onMessageFailure(resip::Handle<resip::InviteSession>, const resip::SipMessage&)
{
}

void SIPEngine::onFailure(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg)
{
}

void SIPEngine::onForkDestroyed(resip::ClientInviteSessionHandle)
{
}

void SIPEngine::onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onProvisional(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg)
{
    Call *call = (Call*)cis->getAppDialogSet().get();

    switch (msg.header(h_StatusLine).statusCode())
    {
    case 100:
        break;
    case 180:
        {
            if (call->mCallState==Initial)
            {
                LOG("Call is ringing");
                call->mCallState = Ringing;            
                AppDialogSetRONACommand cmd(call->getHandle());
                mDum->getSipStack().post(cmd,mRONATimeout,mDum);
            }
        }   
        break;
    }
}

void SIPEngine::onConnected(resip::ClientInviteSessionHandle cis, const resip::SipMessage& msg)
{
}

void SIPEngine::onConnected(resip::InviteSessionHandle is, const resip::SipMessage& msg)
{
    Call *call = (Call*)is->getAppDialogSet().get();
    if (call)
    {
        call->mCallState = Answered;
        Dumais::Sound::RTPSession *rtpSession = call->getRTPSession();
        rtpSession->start();
        mpObserver->onConnectedUas(call);
    }
}

void SIPEngine::onStaleCallTimeout(resip::ClientInviteSessionHandle cis)
{
}

void SIPEngine::onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onAnswer(resip::InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& sdp)
{
    Call *pCall = (Call*)is->getAppDialogSet().get();

    const char* peerIP = sdp.session().origin().getAddress().c_str();
    unsigned int peerPort = sdp.session().media().front().port();

    LOG("Outgoing call was answered. SDP: "<<peerIP<<":"<<peerPort);

    Dumais::Sound::RTPSession *rtpSession = pCall->getRTPSession();
    rtpSession->setPeerAddress(peerIP, peerPort);
    rtpSession->start();

    pCall->mCallState = Answered;
    mpObserver->onAnswer(pCall);

}

void SIPEngine::onEarlyMedia(resip::ClientInviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents& sdp)
{
}

void SIPEngine::onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onOfferRejected(resip::Handle<resip::InviteSession>, const resip::SipMessage *msg)
{
}

void SIPEngine::onInfo(resip::InviteSessionHandle is, const resip::SipMessage& msg)
{
    Call *call = dynamic_cast<Call*>(is->getAppDialogSet().get());
    is->acceptNIT();

    Mime dtmf(resip::Data("application"), resip::Data("dtmf-relay"));
    Contents* contents = msg.getContents();
    Mime mime = msg.getContents()->getType();
    if (mime.type() == "application" && mime.subType() == "dtmf-relay")
    {
        Data data = msg.getContents()->getBodyData();
        std::istringstream iss(data.c_str());
        std::string digit;
        std::getline(iss, digit,'=');
        iss >> digit;

        mpObserver->onDigit(call,digit);
    }
}

void SIPEngine::onRefer(resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onReferAccepted(resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg)
{
}
                                                                                                                                            
void SIPEngine::onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg)
{
}

void SIPEngine::onReferNoSub(resip::Handle<resip::InviteSession>, const resip::SipMessage&)
{
}

void SIPEngine::onRemoved(resip::ClientRegistrationHandle)
{
}

int SIPEngine::onRequestRetry(resip::ClientRegistrationHandle, int retrySeconds, const resip::SipMessage& response)
{
}

void SIPEngine::onNewSession(resip::ServerInviteSessionHandle sis, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg)
{
    Call *call = dynamic_cast<Call*>(sis->getAppDialogSet().get());
    if (call)
    {
        call->mInviteSessionHandle = sis->getSessionHandle();
        call->setIncomming(true);
        call->mFrom = msg.header(h_From);
        call->mTo = msg.header(h_To);

    }

    if (!mpObserver->onNewCallUas(call))
    {
        LOG("Call refused (Busy here)");
        sis->reject(486);
        return;
    }
    
}

void SIPEngine::onNewSession(resip::ClientInviteSessionHandle cis, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg)
{
    Call *call = dynamic_cast<Call*>(cis->getAppDialogSet().get());
    call->mInviteSessionHandle = cis->getSessionHandle();
    mpObserver->onNewCallUac(call);
}

void SIPEngine::onTerminated(resip::InviteSessionHandle is, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg)
{
    Call *call = (Call*)is->getAppDialogSet().get();
    if (call)
    {
        LOG("Call terminated");
        Dumais::Sound::RTPSession *rtp = call->getRTPSession();
        if (rtp)
        {
            rtp->stop();
        }
    }
    
    mpObserver->onCallTerminated(call);
}

void SIPEngine::onOffer(resip::InviteSessionHandle is, const resip::SipMessage& msg, const resip::SdpContents& sdp)
{
    LOG("PhoneModule::onOffer. uas");
    Call *call = dynamic_cast<Call*>(is->getAppDialogSet().get());
    if (call)
    {

        Dumais::Sound::RTPSession *rtpSession = this->mpSoundDeviceFactory->createRTPSession();
        call->setRTPSession(rtpSession);

        const char* peerIP = sdp.session().origin().getAddress().c_str();
        unsigned int peerPort = sdp.session().media().front().port();
        rtpSession->setPeerAddress(peerIP, peerPort);
    }

    answerIncomming(call);

}

void SIPEngine::answerIncomming(Call* call)
{

        Dumais::Sound::RTPSession *rtpSession = call->getRTPSession();

        std::stringstream ss;
        ss<<"v=0\r\n"
            "o=- 0 0 IN IP4 "<<rtpSession->getLocalIP()<<"\r\n"
            "s="<<this->settings.mUserName<<"\r\n"
            "c=IN IP4 "<<rtpSession->getLocalIP()<<"\r\n"
            "t=0 0\r\n"
            "m=audio "<<rtpSession->getLocalPort()<<" RTP/AVP 0 \r\n" // only support G.711 uLaw
            "a=rtpmap:0 pcmu/8000\r\n";
        Data txt(ss.str().c_str());
        HeaderFieldValue hfv(txt.data(), txt.size());
        Mime type("application", "sdp");
        SdpContents sdp2(hfv, type);
        UInt64 currentTime = Timer::getTimeMicroSec();
        sdp2.session().origin().getSessionId() = currentTime;
        sdp2.session().origin().getVersion() = currentTime;


        ServerInviteSession *sis = dynamic_cast<ServerInviteSession*>(call->mInviteSessionHandle.get());
        if (sis)
        {
            sis->provideAnswer(sdp2);
            LOG("Answering incomming call");
            sis->accept();

        }
}


void SIPEngine::onSuccess(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& successResponse)
{
}

void SIPEngine::onFailure(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& errorResponse)
{
}

void SIPEngine::onReceivedRequest(resip::ServerOutOfDialogReqHandle ood, const resip::SipMessage& request)
{
    LOG("PhoneModule::onReceivedRequest");
    if ((request.header(h_ContentType).type() == "application") && (request.header(h_ContentType).subType() == "simple-message-summary"))
    {
        MessageWaitingContents* mwi = dynamic_cast<MessageWaitingContents*>(request.getContents());

        int num = mwi->header(mw_voice).newCount();
        mpObserver->onMWI(num);
        
    }
    
    SharedPtr<SipMessage> resp = ood->accept();
    mDum->send(resp);
}

void SIPEngine::onUpdatePending (ClientSubscriptionHandle, const SipMessage &notify, bool outOfOrder)
{
}

void SIPEngine::onUpdateActive (ClientSubscriptionHandle is, const SipMessage &msg, bool outOfOrder)
{
    is->acceptUpdate();
    if ((msg.header(h_ContentType).type() == "application") && (msg.header(h_ContentType).subType() == "simple-message-summary"))
    { // MWI
        MessageWaitingContents* mwi = dynamic_cast<MessageWaitingContents*>(msg.getContents());

        int num = mwi->header(mw_voice).newCount();
        LOG("Got MWI event " << num);
        mpObserver->onMWI(num);
        return;
    } else if (msg.header(h_ContentType).subType() == "pidf+xml"){ // Presence*/

        std::string presenceStatus="";
        if(msg.getContents())
        {
            Data data = msg.getContents()->getBodyData();
            try
            {
                XMLCursor cursor(ParseBuffer(data.data(),data.size()));
                if (cursor.firstChild())
                {
                    while (cursor.getTag()!="note")
                    {
                        cursor.nextSibling();
                    }
                    if (cursor.firstChild())
                    {
                        presenceStatus = cursor.getValue().c_str();
                    }
                }
            } catch (ParseException &e) {
            }
        }


        Subscription *pSubscription = dynamic_cast<Subscription*>(is->getAppDialogSet().get());
        if (pSubscription)
        {
            if (pSubscription->mDeviceStatus != presenceStatus)
            {
                pSubscription->mDeviceStatus = presenceStatus;
                mpObserver->onPresence(pSubscription);
            }
        }
    }
}

void SIPEngine::onUpdateExtension (ClientSubscriptionHandle, const SipMessage &notify, bool outOfOrder)
{
}

int SIPEngine::onRequestRetry (ClientSubscriptionHandle, int retrySeconds, const SipMessage &notify)
{
    return -1;
}

void SIPEngine::onTerminated (ClientSubscriptionHandle is, const SipMessage *msg)
{
//    LOG("PhoneModule::onTerminated");
    Subscription *pSubscription = dynamic_cast<Subscription*>(is->getAppDialogSet().get());
    if (pSubscription)
    {
        pSubscription->mDeviceStatus = "terminated";
        mpObserver->onPresence(pSubscription);
    }
}

void SIPEngine::onNewSubscription (ClientSubscriptionHandle is, const SipMessage &msg)
{
    Subscription *pSubscription = dynamic_cast<Subscription*>(is->getAppDialogSet().get());
    if (pSubscription)
    {
        mpObserver->onNewDevicePresence(pSubscription);
    }
}



