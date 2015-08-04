#ifndef STARTALLLINKINGMESSAGE_H
#define STARTALLLINKINGMESSAGE_H

#include "IInsteonMessage.h"

class StartAllLinkingMessage: public IInsteonMessage{
private:

public:
    enum LinkType
    {
        Responder,
        Controller
    };

	StartAllLinkingMessage(unsigned char group, LinkType type);
	~StartAllLinkingMessage();

    virtual std::string toString();
    virtual bool needAck();
    virtual InsteonID getDestination();

};

#endif

