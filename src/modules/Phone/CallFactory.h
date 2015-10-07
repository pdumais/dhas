#ifndef CALLFACTORY_H
#define CALLFACTORY_H

#include <resip/dum/AppDialogSetFactory.hxx>
#include "ActionMachine.h"

class CallFactory: public resip::AppDialogSetFactory
{
private:
    ActionMachine *mpActionMachine;
public:
	CallFactory(ActionMachine *am);
	~CallFactory();

    virtual resip::AppDialogSet* createAppDialogSet(resip::DialogUsageManager& dum, const resip::SipMessage& msg);

};

#endif

