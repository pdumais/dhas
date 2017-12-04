#ifndef INSTEONMODEM_H
#define INSTEONMODEM_H
#include "ISerialPort.h"
#include <queue>
#include "InsteonDefs.h"
#include "IInsteonMessageHandler.h"
#include <pthread.h>
#include "IInsteonMessage.h"

class InsteonModem{
private:
    pthread_mutex_t queueLock;
    std::queue<IInsteonMessage*> mInsteonCommandQueue;
    ISerialPort *mpSerialPort;
    time_t mTimeLastSend;
    IInsteonMessageHandler *mpInsteonMessageHandler;
    bool mWaitingForResponse;
    

    void processAllLinkRecordResponse(unsigned char* buf);
    void setIMConfiguration(unsigned char cmd);
public:
	InsteonModem(std::string mac, IInsteonMessageHandler *p);
//	InsteonModem(char *serialPort, IInsteonMessageHandler *p);
	~InsteonModem();

    bool process(bool readyToSend);

    void writeCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2);
    void writeI2CSCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char *buf=0);
    void writeGroupCleanupCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2);
    void writeExtendedCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char* data);
    void waitForUnsolicitedMessage();
    void processStandardMessage(unsigned char *buf);

    void startAllLikingAsController(unsigned char group);
    void startAllLikingAsResponder(unsigned char group);

    bool waitForReply(IInsteonMessage *cmd);
    int  getByte();
    void processEcho(unsigned char* buf);
    void processUnsolicitedEvent(unsigned char *buf);
    bool sendCommand(IInsteonMessage* cmd);
    bool messageResponseWaiting();
    
    void getFirstAllLinkRecord();
    void getNextAllLinkRecord();

};

#endif

