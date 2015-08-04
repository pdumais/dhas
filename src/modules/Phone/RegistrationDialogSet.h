#ifndef ACCOUNTDIALOGSET_H
#define ACCOUNTDIALOGSET_H

#include <resip/dum/AppDialogSet.hxx>
#include "AccountSettings.h"

class RegistrationDialogSet: public resip::AppDialogSet{
public:
	RegistrationDialogSet(resip::DialogUsageManager&);
	~RegistrationDialogSet();

    void setAccountSettings(AccountSettings settings);

    const AccountSettings getAccountSettings();

private:
    AccountSettings mSettings;
};

#endif

