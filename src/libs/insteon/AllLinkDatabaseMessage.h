#ifndef ALLLINKDATABASEMESSAGE_H
#define ALLLINKDATABASEMESSAGE_H
#include "IInsteonMessage.h"

class AllLinkDatabaseMessage: public IInsteonMessage{
private:

public:
	AllLinkDatabaseMessage(bool first=true);
	~AllLinkDatabaseMessage();
    
    virtual std::string toString();
    virtual InsteonID getDestination();
    virtual bool needAck();

};

#endif

