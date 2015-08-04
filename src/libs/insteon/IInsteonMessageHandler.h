#pragma once
#include "InsteonDefs.h"
#include "IInsteonMessage.h"

class IInsteonMessageHandler
{
public:
    virtual void onInsteonMessage(unsigned char *buf)=0;
    virtual void onInsteonAllLinkRecordResponse(unsigned char *buf)=0;
    virtual void onInsteonMessageSent(InsteonID id, IInsteonMessage *cmd) =0;
};
