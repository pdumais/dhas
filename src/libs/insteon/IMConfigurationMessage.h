#ifndef IMCONFIGURATIONMESSAGE_H
#define IMCONFIGURATIONMESSAGE_H
#include "IInsteonMessage.h"

class IMConfigurationMessage: public IInsteonMessage
{
private:

public:
	IMConfigurationMessage(unsigned char config);
	~IMConfigurationMessage();

    virtual std::string toString();
    virtual InsteonID getDestination();
    virtual bool needAck();

};

#endif

