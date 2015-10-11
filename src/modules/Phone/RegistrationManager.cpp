#include "DHASLogging.h"
#include <resip/dum/ClientRegistration.hxx>
#include "RegistrationManager.h"

using namespace resip;

RegistrationManager::RegistrationManager(){
    this->mDum = 0;
    this->mRegistrationDialogSet = 0;
}

RegistrationManager::~RegistrationManager(){
    if (this->mRegistrationDialogSet) {
        delete this->mRegistrationDialogSet;
    }

}

void RegistrationManager::setDum(resip::DialogUsageManager *dum)
{
    this->mDum = dum;
}

void RegistrationManager::addObserver(RegistrationObserver *obs)
{
   this->mObservers.push_back(obs); 
}

void RegistrationManager::removeObserver(RegistrationObserver *obs)
{
    //TODO
//   this->observers.push_back(obs);
}


void RegistrationManager::registerAccount(AccountSettings settings)
{
    if (!this->mDum) return;


    if (this->mRegistrationDialogSet) {
        // TODO: must unregister. Should we delete the dialogSet?
    }
    this->mRegistrationDialogSet = new RegistrationDialogSet(*this->mDum);
    this->mRegistrationDialogSet->setAccountSettings(settings);

    NameAddr uacAor;
    std::string uri = "sip:" + settings.mUserName + "@" + settings.mProxy;
    uacAor = NameAddr(Uri(uri.c_str()));

    SharedPtr<SipMessage> msg = this->mDum->makeRegistration(uacAor, this->mRegistrationDialogSet);
    this->mDum->send(msg);   
}

void RegistrationManager::onSuccess(resip::ClientRegistrationHandle handle, const resip::SipMessage& response)
{
    RegistrationDialogSet* ds = (RegistrationDialogSet*)handle->getAppDialogSet().get();
    AccountSettings settings = ds->getAccountSettings();

    this->notifyObservers(true);
}

void RegistrationManager::onRemoved(resip::ClientRegistrationHandle, const resip::SipMessage& response)
{
}

int  RegistrationManager::onRequestRetry(resip::ClientRegistrationHandle, int retrySeconds, const resip::SipMessage& response)
{
}

void RegistrationManager::onFailure(resip::ClientRegistrationHandle, const resip::SipMessage& response)
{
    this->notifyObservers(false);
}

void RegistrationManager::onFlowTerminated(resip::ClientRegistrationHandle)
{
}

void RegistrationManager::notifyObservers(bool registered)
{
    for (std::list<RegistrationObserver*>::iterator it=this->mObservers.begin();it!=this->mObservers.end();it++)
    {
        (*it)->onRegistered(registered);
    }
}
