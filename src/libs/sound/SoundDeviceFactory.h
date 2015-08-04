#pragma once

#include "RTPSession.h"
#include "SoundCard.h"

namespace Dumais{
namespace Sound{

class SoundDeviceFactory{
private:
    unsigned int mNextPort;
    unsigned int mLowPort;
    unsigned int mHighPort;
    std::string mLocalIP;
    bool usingRTP;
public:
	SoundDeviceFactory(unsigned int lport, unsigned int hport);
	SoundDeviceFactory();
	~SoundDeviceFactory();
   
    SoundCard* createSoundCard(const std::string& deviceName);
    RTPSession* createRTPSession();
    void setLocalIP(std::string localIP);
};

}
}


