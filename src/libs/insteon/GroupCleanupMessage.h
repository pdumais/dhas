#ifndef GROUPCLEANUPMESSAGE_H
#define GROUPCLEANUPMESSAGE_H
#include "InsteonDefs.h"
#include "IInsteonMessage.h"

class GroupCleanupMessage: public IInsteonMessage{
private:

public:
	GroupCleanupMessage(InsteonID destination, unsigned char cmd1, unsigned char cmd2);
	~GroupCleanupMessage();

    virtual std::string toString();

    unsigned char command1();
    unsigned char command2();
    void copyData(char *buf);
    virtual InsteonID getDestination();
    virtual bool needAck();



};

#endif

