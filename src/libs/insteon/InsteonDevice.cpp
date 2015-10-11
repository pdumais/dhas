#include "utils/Logging.h"
#include "InsteonDevice.h"

InsteonDevice::InsteonDevice(std::string name, InsteonID id, InsteonModem* p)
{
    mName = name;
    mID = id;
    mInitialized = false;
    mpInsteonModem = p;
    mTransmissionStatus = Idle;
    mLastMessageTimeStamp = true;
}


InsteonDevice::~InsteonDevice(){
}

void InsteonDevice::onTimer(time_t t)
{
}

bool InsteonDevice::isIdle()
{
    time_t t;
    time(&t);

    if ((t-mLastMessageTimeStamp)>=2 && (mTransmissionStatus!=Idle))
    {
        LOG("Message timeout for device " << std::hex << mID << std::dec << " (last=" << mLastMessageTimeStamp << ", now=" << t);
        // it has been 2 seconds since we last sent a message, consider it timed out.
        mTransmissionStatus=Idle;
        mLastMessageTimeStamp=0;
    }

    return (mTransmissionStatus==Idle);
}

std::string InsteonDevice::getName()
{
    return mName;
}

InsteonID InsteonDevice::getID()
{
    return mID;
}

InsteonDirectMessage InsteonDevice::getLastDirectMessageSent()
{
    return mLastDirectMessageSent;
}

void InsteonDevice::setLastDirectMessageSent(InsteonDirectMessage cmd)
{
    time(&mLastMessageTimeStamp);
    mLastDirectMessageSent = cmd;
    mTransmissionStatus = WaitingAckOfDirectMessage;
}

bool InsteonDevice::isInitialized()
{
    return mInitialized;
}

void InsteonDevice::setInitialized()
{
    mInitialized = true;
}

void InsteonDevice::setDeviceParameter(unsigned char param)
{
    mDeviceParameter = param;
}

void InsteonDevice::writeCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2)
{
    if (!mpInsteonModem) return;

    mpInsteonModem->writeCommand(destination,cmd1,cmd2);
}

void InsteonDevice::writeGroupCleanupCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2)
{
    if (!mpInsteonModem) return;

    mpInsteonModem->writeGroupCleanupCommand(destination,cmd1,cmd2);
}

void InsteonDevice::setGroup(unsigned char group, unsigned char data1,unsigned char data2,unsigned char data3)
{
   // Logging::log("setgroup: group=%x, d1=%x, d2=%x, d3=%x",group,data1,data2,data3);
}

void InsteonDevice::writeI2CSCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char *buf)
{
    if (!mpInsteonModem) return;

    mpInsteonModem->writeI2CSCommand(destination,cmd1,cmd2,buf);
}

void InsteonDevice::writeExtendedCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char* data)
{
    if (!mpInsteonModem) return;

    mpInsteonModem->writeExtendedCommand(destination,cmd1,cmd2,data);
}

void InsteonDevice::onInsteonMessage(Dumais::JSON::JSON& json, unsigned char* buf)
{
    unsigned char flags = buf[8];
    unsigned char status = (flags & 0b11100000)>>5;
    unsigned char cmd1 = buf[9];

    // This is an ACK of a direct message
    if (status==0b001)
    {
        // IF the ACK is not for the last message that we sent, we should ignore it because 
        // we never allow to send a command without getting an ACK first. So it means that we are guaranteed that
        // the last command processed its ACK. so if this is an ACK for an old command, we can discard it. 
        // normally, if we get a duplicate ACK, mLastDirectMessageSent.cmd1 will have been set to zero. 
        // But there is acase where we send command A, get ACK for cmd A, send cmd B, get a duplicate ACK for cmd A before
        // getting the ACK for cmd B. In that case, the "cmd1" member will not match.
        if (this->mLastDirectMessageSent.cmd1!=cmd1)
        {
            // in the case where last command was 0x19, the ACK will have a cmd1 of 0x00. So they wont match and we will get here.
            // but we don't want to ignore the ACK. TODO: This should be more robust
            if (this->mLastDirectMessageSent.cmd1 != 0x19)
            {
                // we got an ACK of a direct message but we did not send a message.
                // It is probably a retransmit. Ignore it.
                LOG("Ignoring duplicate ACK of direct message");
                return;
            }
        }
    }

    this->processEvent(json, buf);

    if (status == 0b001)
    {
        // This is an ACK of a direct message.
        this->mTransmissionStatus = Idle;
        this->mLastDirectMessageSent.cmd1=0;
    }
    else if (status == 0b101)
    {
        // This is an NAK of a direct message.
        LOG("ERROR: NAK of a direct message");
        this->mTransmissionStatus = Idle;
    }

}
