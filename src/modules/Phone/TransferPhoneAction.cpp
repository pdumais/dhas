#include "TransferPhoneAction.h"
#include "Call.h"
#include "SIPEngine.h"

TransferPhoneAction::TransferPhoneAction(SIPEngine* engine, const std::string& destination): IPhoneAction(engine)
{
    this->mDestination = destination;
}

void TransferPhoneAction::invoke(Call* call)
{
    mpSIPEngine->transferCall(call, this->mDestination);
}
