#pragma once

class SIPEngine;
class Call;

class IPhoneAction
{
protected:
    SIPEngine *mpSIPEngine;
public:
    IPhoneAction(SIPEngine* engine);
    ~IPhoneAction(){};

    virtual void invoke(Call* call)=0;
};
