#ifndef PHONESERVICE_H
#define PHONESERVICE_H

#include "Module.h"
#include "SIPEngine.h"

typedef std::map<Subscription*,std::string> BLFList;

class PhoneModule: public Module, public TelephonyObserver
{
private:
    SIPEngine* mpSIPEngine;
    std::map<std::string,Call*> mCallsList;
    BLFList mBLFList;
    int mMWI;
    std::string mSoundsFolder;
    Dumais::JSON::JSON mConfig;

    Call* call(std::string destination,std::string playString, bool hangupAfterSounds=true);
    Call* click2dial(std::string source, std::string destination,std::string playString);
    void playOnCall(std::string id, std::string playstring, bool hangupAfterSounds=false);
    void subscribeBLF(std::string device);
    void subscribeMWI(std::string device);
    void getCallsList(Dumais::JSON::JSON &json);
    void getBLFList(Dumais::JSON::JSON &json);
    void releaseCall(std::string id);
    void registerUserAgent(std::string user, std::string pin, std::string proxy);

protected:
    virtual void configure(Dumais::JSON::JSON& config);
public:
	PhoneModule();
	~PhoneModule();

    virtual void run();
    virtual std::string getName(){return "phone";}
    virtual void stop();

    void registerCallBacks(RESTEngine* pEngine);

    void click2dial_callback(RESTContext* context);
    void call_callback(RESTContext* context);
    void register_callback(RESTContext* context);
    void blf_callback(RESTContext* context);
    void showcalls_callback(RESTContext* context);
    void showblf_callback(RESTContext* context);
    void release_callback(RESTContext* context);
    void play_callback(RESTContext* context);


    int getMWI();

    virtual void onMWI(int num);
    virtual void onConnectedUas(Call *pCall);
    virtual void onAnswer(Call *pCall);
    virtual void onDigit(Call *pCall, std::string digit);
    virtual bool onNewCallUas(Call *pCall);
    virtual bool onNewCallUac(Call *pCall);
    virtual void onCallTerminated(Call *pCall);
    virtual void onPresence(Subscription *sub);
    virtual void onNewDevicePresence(Subscription *sub);
};

#endif

