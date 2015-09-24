#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <resip/dum/AppDialogSet.hxx>
#include "json/JSON.h"

class Subscription: public resip::AppDialogSet{
public:
	Subscription(resip::DialogUsageManager &dum);
	~Subscription();

    resip::DialogUsageManager& getDUM();
    std::string getDevice();
    resip::NameAddr getContact();
    std::string getID();
    std::string getDeviceState();
    void toJSON(Dumais::JSON::JSON& json);
    

private:
    // TODO: will need to unfriend this
    friend class PhoneModule;
    friend class SIPEngine;

    resip::InviteSessionHandle mInviteSessionHandle;
    std::string mDevice;
    resip::NameAddr mOwner;
    resip::NameAddr mTo;
    resip::NameAddr mContact;
    std::string mDeviceStatus;
};

#endif

