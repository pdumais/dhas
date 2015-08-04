#ifndef CALLFACTORY_H
#define CALLFACTORY_H

#include <resip/dum/AppDialogSetFactory.hxx>

class CallFactory: public resip::AppDialogSetFactory
{
private:

public:
	CallFactory();
	~CallFactory();

    virtual resip::AppDialogSet* createAppDialogSet(resip::DialogUsageManager& dum, const resip::SipMessage& msg);

};

#endif

