#ifndef STANDARDOREXTENDEDMESSAGE_H
#define STANDARDOREXTENDEDMESSAGE_H
#include "InsteonDefs.h"
#include "IInsteonMessage.h"

class StandardOrExtendedMessage: public IInsteonMessage{
private:

public:
	StandardOrExtendedMessage(InsteonID destination, unsigned char cmd1, unsigned char cmd2);
    StandardOrExtendedMessage(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char *data,bool i2cs=false);
	~StandardOrExtendedMessage();

    virtual std::string toString();

    bool isExtended();
    unsigned char command1();
    unsigned char command2();
    void copyData(char *buf);
    virtual InsteonID getDestination();
    virtual bool needAck();



};

#endif

