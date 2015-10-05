#include "Logging.h"
#include "PhoneModule.h"
#include "ReleasePhoneAction.h"
#include "TransferPhoneAction.h"
#include "SoundListParser.h"
#include "ModuleRegistrar.h"
#include <resip/dum/ClientSubscription.hxx>

#define PLAYSTRING_DESCRIPTION "Coma separated list of files to play. digits can be used. They will be decoded and the proper sound files will be constructed. I.e: sound1,29,4,sound2 would play sound files: sound1.wav,20.wav,9.wav,4.wav,sound2.wav Note that number reconstruction only work for french grammar and only supports numbers -59..59 inclusively. For playing silence, you can use the number of seconds prefixed by a dollar sign. I.e: sound1.wav,$4,sound2.wav. This would make a 4 second pause between sound1 and sound2"

using namespace resip;

REGISTER_MODULE(PhoneModule)

PhoneModule::PhoneModule()
{
    mMWI = 0;
    mpSIPEngine = 0;
}

PhoneModule::~PhoneModule()
{
    if (mpSIPEngine) delete mpSIPEngine;
}

void PhoneModule::configure(Dumais::JSON::JSON& config)
{
    mConfig = config;
}

void PhoneModule::registerCallBacks(RESTEngine* pEngine)
{
    std::string tmp;
    RESTCallBack *p;

    p = new RESTCallBack(this,&PhoneModule::call_callback,"Will call the given extension and optionally play sound when the remote peer answers the call. Placing a call only works if the user agent was previously registered. Called extension must be know by the proxy because direct URI are not supported. To make an intercom call (where the UAS will autoanswer) this needs to be configured on the proxy. ");
    p->addParam("ext","extention to call");
    p->addParam("play",PLAYSTRING_DESCRIPTION);
    pEngine->addCallBack("/phone/call","GET",p);

    p = new RESTCallBack(this,&PhoneModule::register_callback,"Will register the phone service user agent to the given PBX. This is usually done during initialization ");
    p->addParam("user","SIP user to register");
    p->addParam("pin","pin associated to user");
    p->addParam("proxy","PBX IP address");
    pEngine->addCallBack("/phone/register","GET",p);

    p = new RESTCallBack(this,&PhoneModule::blf_callback,"Will subscribe for presence events for the given extension. The extension must be a known extension in the subscribe context of our UA (if using Asterisk). ");
    p->addParam("ext","extension");
    pEngine->addCallBack("/phone/blf","GET",p);

    p = new RESTCallBack(this,&PhoneModule::showcalls_callback,"Get the list of active calls in the system ");
    pEngine->addCallBack("/phone/showcalls","GET",p);

    p = new RESTCallBack(this,&PhoneModule::showblf_callback,"Get the list of active subscriptions to presence events in the system ");
    pEngine->addCallBack("/phone/showblf","GET",p);

    p = new RESTCallBack(this,&PhoneModule::release_callback,"release a call using call ID (usually provided in call events) ");
    p->addParam("id","Call ID");
    pEngine->addCallBack("/phone/release","GET",p);

    p = new RESTCallBack(this,&PhoneModule::play_callback,"Play sounds on an active call using given callID.");
    p->addParam("sound",PLAYSTRING_DESCRIPTION);
    p->addParam("id","call ID");
    p->addParam("releaseaftersounds","[true/false] if you want the call to be released after sound finished playing");
    pEngine->addCallBack("/phone/play","GET",p);

    tmp = "Sound to play when source answers, before transfering to destination. ";
    tmp += PLAYSTRING_DESCRIPTION;
    p = new RESTCallBack(this,&PhoneModule::click2dial_callback,"Click-2-Dial");
    p->addParam("sound",tmp);
    p->addParam("src","Source extension to call");
    p->addParam("dst","Destination extension to which the source will be patched.");
    pEngine->addCallBack("/phone/click2dial","GET",p);

}


void PhoneModule::click2dial_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    this->click2dial(params->getParam("src"),params->getParam("dst"),params->getParam("sound"));

}

void PhoneModule::call_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    bool releaseAfterSounds = true;
    if (params->getParam("releaseaftersounds")=="false") releaseAfterSounds = false;
    this->call(params->getParam("ext"),params->getParam("play"),releaseAfterSounds);

}

void PhoneModule::register_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    this->registerUserAgent(params->getParam("user"),params->getParam("pin"),params->getParam("proxy"));
}

void PhoneModule::blf_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    this->subscribeBLF(params->getParam("ext"));
}

void PhoneModule::showcalls_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    this->getCallsList(json);
}

void PhoneModule::showblf_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    this->getBLFList(json);
}

void PhoneModule::release_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    this->releaseCall(params->getParam("id"));
}

void PhoneModule::play_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    bool releaseAfterSounds = true;
    if (params->getParam("releaseaftersounds")=="false") releaseAfterSounds = false;
    this->playOnCall(params->getParam("id"),params->getParam("sound"),releaseAfterSounds);
}

void PhoneModule::onMWI(int num)
{
    mMWI = num;
}

void PhoneModule::onAnswer(Call *pCall)
{
    SoundListParser parser(pCall->mPlayString);
    std::vector<std::string> list = parser.getSoundList();
    for (std::vector<std::string>::iterator it = list.begin();it!=list.end();it++)
    {
        std::string sound = *it;
        if (sound[0]=='$')
        {
            std::string st = sound.substr(1,std::string::npos);
            int duration = atoi(st.c_str());
            Logging::log("Queuing %i seconds silence on call \r\n",duration);
            pCall->getRTPSession()->silence(duration);
        } else {
            std::string st = this->mSoundsFolder+(sound)+"-ulaw.wav";
            Logging::log("Queuing %s on call \r\n",st.c_str());
            pCall->getRTPSession()->play(st.c_str(), Dumais::Sound::G711);
        }
    }
}

void PhoneModule::onNewDevicePresence(Subscription *pSub)
{
    //Logging::log("PhoneModule::onNewDevicePresence: %i",pSub);

    // After expiry, we will get here but it will still be the same Subscription instance
    if (mBLFList.find(pSub)==mBLFList.end())
    {
        // it is not in the list. Add it.
        mBLFList[pSub]="unknown";
    }
}

int PhoneModule::getMWI()
{
    return mMWI;
}

void PhoneModule::onPresence(Subscription *pSub)
{
    BLFList::iterator it = mBLFList.find(pSub);
    if (it!=mBLFList.end())
    {
        if (pSub->getDeviceState()!=it->second) // dont send event if status is the same
        {
            mBLFList[pSub] = pSub->getDeviceState(); 
            Dumais::JSON::JSON json;
            json.addValue("presence","event");
            json.addValue(pSub->getDevice(),"device");
            json.addValue(pSub->getDeviceState(),"status");
            mpEventProcessor->processEvent(json);

            Logging::log("Presence: device %s state is %s",pSub->getDevice().c_str(),pSub->getDeviceState().c_str());

            if (pSub->getDeviceState()=="terminated")
            {
                mBLFList.erase(it);
            }
        }
    }
}


void PhoneModule::onCallTerminated(Call *call)
{
    std::map<std::string,Call*>::iterator it = mCallsList.find(call->getID());
    if (it!=mCallsList.end()) mCallsList.erase(it);

    Dumais::JSON::JSON json;
    json.addValue("call","event");
    json.addObject("call");
    json["call"].addValue(call->mFrom.uri().user().data(),"from");
    json["call"].addValue(call->mTo.uri().user().data(),"to");
    json["call"].addValue(call->getID(),"id");
    json.addObject("callevent");
    json["callevent"].addValue("released","type");
    json["callevent"].addValue(call->isIncomming()?"incoming":"outgoing","dir");
    mpEventProcessor->processEvent(json);
}


bool PhoneModule::onNewCallUas(Call *call)
{
    Logging::log("Incoming call\r\n");
    for (std::map<std::string,Call*>::iterator it=mCallsList.begin();it!=mCallsList.end();it++)
    {
        // Only allow one incomming call at a atime
        if (it->second->isIncomming())
        {
            return false;
        }
    }
    call->addRTPObserver(this);
    mCallsList[call->getID()]=call;
    return true;
}



bool PhoneModule::onNewCallUac(Call *call)
{
    mCallsList[call->getID()]=call;
    return true;
}

void PhoneModule::onDigit(Call *call, std::string digit)
{
    // when we press a digit, we must clear the sound queue
    call->getRTPSession()->clearQueue();

    Dumais::JSON::JSON json;
    json.addValue("call","event");
    json.addObject("call");
    json["call"].addValue(call->mFrom.uri().user().data(),"from");
    json["call"].addValue(call->mTo.uri().user().data(),"to");
    json["call"].addValue(call->getID(),"id");
    json.addObject("callevent");
    json["callevent"].addValue("digit","type");
    json["callevent"].addValue(digit,"digit");
    mpEventProcessor->processEvent(json);


}

void PhoneModule::onConnectedUas(Call *call)
{
    Dumais::JSON::JSON json;
    json.addValue("call","event");
    json.addObject("call");
    json["call"].addValue(call->mFrom.uri().user().data(),"from");
    json["call"].addValue(call->mTo.uri().user().data(),"to");
    json["call"].addValue(call->getID(),"id");
    json.addObject("callevent");
    json["callevent"].addValue("answered","type");
    json["callevent"].addValue("incoming","dir");
    mpEventProcessor->processEvent(json);
}

void PhoneModule::subscribeBLF(std::string device)
{
    mpSIPEngine->subscribeBLF(device);
}

void PhoneModule::subscribeMWI(std::string device)
{
    mpSIPEngine->subscribeMWI(device);
}

void PhoneModule::getCallsList(Dumais::JSON::JSON& json)
{
    //TODO: This will be called from another thread. Make it safe
    json.addList("calls");
    for (std::map<std::string,Call*>::iterator it = mCallsList.begin();it!=mCallsList.end();it++)
    {
        Call *pCall = it->second;
        pCall->toJSON(json["calls"]);
    }

}

void PhoneModule::getBLFList(Dumais::JSON::JSON& json)
{
    //TODO: This will be called from another thread. Make it safe
    json.addList("subscriptions");
    for (BLFList::iterator it = mBLFList.begin();it!=mBLFList.end();it++)
    {
        Subscription *pSub = it->first;
        pSub->toJSON(json["subscriptions"]);
    }

}


void PhoneModule::releaseCall(std::string id)
{
    //TODO: This will be called from another thread. Make it safe

    std::map<std::string,Call*>::iterator it = mCallsList.find(id);
    if (it!=mCallsList.end())
    {
        mpSIPEngine->releaseCall(it->second);
    }
}

void PhoneModule::playOnCall(std::string id, std::string playstring, bool hangupAfterSounds)
{
    //TODO: This will be called from another thread. Make it safe
    std::map<std::string,Call*>::iterator it = mCallsList.find(id);
    if (it!=mCallsList.end())
    {
        Call *pCall = it->second;
        if (hangupAfterSounds)
        {
           pCall->mAfterSoundsHandler = new ReleasePhoneAction(mpSIPEngine);
        }

//        pCall->mHangupAfterSounds = hangupAfterSounds;
        SoundListParser parser(playstring);
        std::vector<std::string> list = parser.getSoundList();
        for (std::vector<std::string>::iterator it = list.begin();it!=list.end();it++)
        {
            std::string sound = *it;
            if (sound[0]=='$')
            {
                std::string st = sound.substr(1,std::string::npos);
                int duration = atoi(st.c_str());
                Logging::log("Queuing %i seconds silence on call \r\n",duration);
                pCall->getRTPSession()->silence(duration);
            } else {
                std::string st = this->mSoundsFolder+(*it)+"-ulaw.wav";
                Logging::log("Queuing %s on call \r\n",st.c_str());
                pCall->getRTPSession()->play(st.c_str(), Dumais::Sound::G711);
            }
        }
    }

}

Call* PhoneModule::click2dial(std::string source, std::string destination,std::string playString)
{
    //TODO: This will be called from another thread. Make it safe
    Call *call = mpSIPEngine->makeCall(source);
    if (call==0) return 0;

    call->mAfterSoundsHandler = new TransferPhoneAction(mpSIPEngine, destination);
    //call->mHangupAfterSounds = false;
    call->setPlayString(playString);
    call->addRTPObserver(this);

    return call;
}

Call* PhoneModule::call(std::string destination,std::string playString, bool hangupAfterSounds)
{

    //TODO: This will be called from another thread. Make it safe
    Call *call = mpSIPEngine->makeCall(destination);
    if (call==0) return 0;

    if (hangupAfterSounds)
    {
       call->mAfterSoundsHandler = new ReleasePhoneAction(mpSIPEngine);
    }
//    call->mHangupAfterSounds = hangupAfterSounds;
    call->setPlayString(playString);
    call->addRTPObserver(this);

    return call;
}




void PhoneModule::run()
{
    this->mSoundsFolder = mConfig["soundsfolder"].str();
    mpSIPEngine = new SIPEngine(mConfig["rtplow"].toInt(),
        mConfig["rtphigh"].toInt(),
        mConfig["localip"].str(),
        mConfig["sipport"].toInt(),
        mConfig["ronatimeout"].toInt());
    mpSIPEngine->setTelephonyObserver(this);
    mpSIPEngine->start();
    setStarted();
    while (!stopping())
    {
        mpSIPEngine->run();
    }
    Logging::log("SIP thread ending");
}

void PhoneModule::stop()
{
    mpSIPEngine->stop();
}

void PhoneModule::registerUserAgent(std::string user, std::string pin, std::string proxy)
{
    //TODO: This will be called from another thread. Make it safe
    AccountSettings settings;
    settings.mUserName = user;
    settings.mPin = pin;
    settings.mProxy = proxy;
    settings.mDisplayName = user;
    mpSIPEngine->registerUserAgent(settings);

}


void PhoneModule::onRTPSessionSoundQueueEmpty(Call *call)
{
    Logging::log("RTP Sound Queue Empty");
    call->invokeAfterSoundsHandler();
    /*if (call->mHangupAfterSounds)
    {
        mpSIPEngine->releaseCall(call);
    }*/
}



