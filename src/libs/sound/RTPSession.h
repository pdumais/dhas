#ifndef WAKEUPRTPSESSION_H
#define WAKEUPRTPSESSION_H

#define CONF_RTP_PAYLOAD_SIZE 160

#include <ortp/ortp.h>
#include <string>
#include <queue>
#include "ISound.h"
#include "ISoundDevice.h"
#include <list>
#include <thread>

namespace Dumais{
namespace Sound{

class RTPSession: public Dumais::Sound::ISoundDevice
{
private:
    RtpSession *mSession;
    std::string mPeerIP;
    unsigned int mPeerPort;
    std::string mLocalIP;
    unsigned int mLocalPort;
    unsigned int mUser_ts;
    unsigned char mBuffer[CONF_RTP_PAYLOAD_SIZE];
    bool mResetTime;
    bool mInterruptCurrentSound;
    void work();
protected:
    virtual unsigned int getSampleRate() { return 8000; }
    virtual unsigned int getSampleSize() { return 1; }

public:
	RTPSession();
	~RTPSession();

    void start();
    void stop();

    void setPeerAddress(std::string ip, unsigned int peerPort);
    void setLocalAddress(std::string ip, unsigned int port);    
    unsigned int getLocalPort();
    std::string getLocalIP();
    

};

}
}

#endif

