#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include <string>

class AccountSettings{
private:

public:
	AccountSettings();
    AccountSettings(const AccountSettings& settings);
	~AccountSettings();

    std::string mUserName;
    std::string mPin;
    std::string mProxy;
    std::string mDisplayName;

};

#endif

