#include "Logging.h"
#include "RegistrationDialogSet.h"

RegistrationDialogSet::RegistrationDialogSet(resip::DialogUsageManager &dum):AppDialogSet(dum){
}

RegistrationDialogSet::~RegistrationDialogSet(){
}

void RegistrationDialogSet::setAccountSettings(AccountSettings settings)
{
    this->mSettings = settings;
}

const AccountSettings RegistrationDialogSet::getAccountSettings()
{
    return this->mSettings;
}

