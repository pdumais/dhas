#include "SoundDeviceFactory.h"
#include "stdio.h"

using namespace Dumais::Sound;

SoundDeviceFactory::SoundDeviceFactory(unsigned int lport, unsigned int hport)
{
    this->mHighPort = hport;
    this->mLowPort = lport;
    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
    this->mNextPort = this->mLowPort;
    this->usingRTP = true;
}

SoundDeviceFactory::SoundDeviceFactory()
{
    this->mHighPort = 0;
    this->mLowPort = 0;
    this->mNextPort = 0;
    this->usingRTP = false;
}

SoundDeviceFactory::~SoundDeviceFactory()
{
    if (this->usingRTP) ortp_exit();
}

RTPSession* SoundDeviceFactory::createRTPSession()
{
    if (!this->usingRTP) return 0;

    RTPSession *rtpSession = new RTPSession();
    rtpSession->setLocalAddress(this->mLocalIP,this->mNextPort);
    this->mNextPort+=2;
    if (this->mNextPort>=this->mHighPort)
    {
        this->mNextPort = this->mLowPort;
    }

    return rtpSession;
}

void SoundDeviceFactory::setLocalIP(std::string localIP)
{
    if (!this->usingRTP) return;
    this->mLocalIP = localIP;
}

SoundCard* SoundDeviceFactory::createSoundCard(const std::string& deviceName)
{
    return new SoundCard(deviceName);
}
