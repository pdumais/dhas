#include "ReleasePhoneAction.h"
#include "Call.h"
#include "SIPEngine.h"

ReleasePhoneAction::ReleasePhoneAction(SIPEngine* engine): IPhoneAction(engine)
{
}

void ReleasePhoneAction::invoke(Call* call)
{
    mpSIPEngine->releaseCall(call);
}
