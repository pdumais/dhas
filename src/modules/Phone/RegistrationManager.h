#ifndef REGISTRATIONMANAGER_H
#define REGISTRATIONMANAGER_H

#include <list>
#include <resip/dum/RegistrationHandler.hxx>
#include <resip/dum/DialogUsageManager.hxx>
#include "RegistrationDialogSet.h"
#include "RegistrationObserver.h"

class RegistrationManager: public resip::ClientRegistrationHandler{
private:
    std::list<RegistrationObserver*> mObservers;

public:
	RegistrationManager();
	~RegistrationManager();

    void registerAccount(AccountSettings settings);
    void setDum(resip::DialogUsageManager *dum);

    void onSuccess(resip::ClientRegistrationHandle, const resip::SipMessage& response);
    void onRemoved(resip::ClientRegistrationHandle, const resip::SipMessage& response);
    int  onRequestRetry(resip::ClientRegistrationHandle, int retrySeconds, const resip::SipMessage& response);
    void onFailure(resip::ClientRegistrationHandle, const resip::SipMessage& response);
    void onFlowTerminated(resip::ClientRegistrationHandle);
    void addObserver(RegistrationObserver *obs);
    void removeObserver(RegistrationObserver *obs);

        

private:
    resip::DialogUsageManager *mDum;
    RegistrationDialogSet *mRegistrationDialogSet;

    void notifyObservers(bool registered);

};

#endif

