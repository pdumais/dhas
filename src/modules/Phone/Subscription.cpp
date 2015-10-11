#include "DHASLogging.h"
#include "Subscription.h"
#include <stdio.h>

#include "resip/dum/InviteSession.hxx"
#include "resip/dum/ClientInviteSession.hxx"

using namespace resip;

Subscription::Subscription(resip::DialogUsageManager &dum):AppDialogSet(dum)
{
}

Subscription::~Subscription()
{
}

DialogUsageManager& Subscription::getDUM()
{
    return this->mDum;
}

std::string Subscription::getDeviceState()
{
    return mDeviceStatus;
}

std::string Subscription::getDevice()
{
    return mDevice;
}

resip::NameAddr Subscription::getContact()
{
    return mContact;
}

std::string Subscription::getID()
{
    return this->getDialogSetId().getCallId().c_str();
}

void Subscription::toJSON(Dumais::JSON::JSON& json)
{
    Dumais::JSON::JSON& j = json.addObject("subscription");
    j.addValue(getID(),"id");
    j.addValue(mDevice,"device");
    j.addValue(mDeviceStatus,"devicestate");
}
