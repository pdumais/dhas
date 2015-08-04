#include "InsteonLogging.h"
#include "SwitchDevice.h"
#include <iomanip>
SwitchDevice::SwitchDevice(std::string name, InsteonID id, InsteonModem *p): InsteonDevice(name,id,p)
{
    mDeviceParameter = 0;

    // Query that device's initial status
    this->writeCommand(id,0x19,0);
}

SwitchDevice::~SwitchDevice(){
}

unsigned char SwitchDevice::getLevel()
{
    return mDeviceParameter;
}

/*void SwitchDevice::setLevel(unsigned char v)
{
    mDeviceParameter = v;
}*/

void SwitchDevice::toggle(unsigned char subdev)
{
    if (mDeviceParameter == 0)
    {
        this->writeCommand(mID,0x11,255);
    } 
    else 
    {
        this->writeCommand(mID,0x14,0);
    }
}

void SwitchDevice::turnOnOrOff(bool on, unsigned char level, unsigned char subdev)
{
    if (on)
    {
       unsigned char cmd = 0x11;
       if (level ==255)
       {
           cmd = 0x12;
       } else {
           cmd = 0x11;
       }
       this->writeCommand(mID,cmd,level);
    } 
    else
    {
        this->writeCommand(mID,0x14,0);
    }
}

InsteonDeviceType SwitchDevice::getDeviceType()
{
    return OnOff;
}

void SwitchDevice::processEvent(Dumais::JSON::JSON& json, unsigned char* buf)
{
    // Get the status
    unsigned char flags = buf[8];
    unsigned char status = (flags & 0b11100000)>>5;
    unsigned char cmd1 = buf[9];
    unsigned char cmd2 = buf[10];
    bool ackOfDirectMessage = (status==0b001);

    if (ackOfDirectMessage)
    {
        unsigned char command = cmd1;
        if (getLastDirectMessageSent().cmd1==0x19) command = 0x19;
        // The "status request" command does not echo the command in the message. We must deduce it.
        // So we check what was the last message sent to associate this ACK to it.
        if (command==0x19)
        {
             // Note: It is possible that we get level=0. In this case, the light is off.
            this->setDeviceParameter(cmd2);
            if (this->isInitialized())
            {

                json.addValue("insteon","event");
                json.addValue(mID,"id");
                json.addValue(this->getName(),"name");
                json.addValue("unsolicited","trigger");
                json.addValue(this->getDeviceType(),"type");
                json.addValue((unsigned int)cmd2,"value");
            }
            else
            {
                this->setInitialized();
                // The first time we get this, we don't wanna notify anyone.
                LOG("Initial level for device "<<std::hex<<mID<<" is " << cmd2);
            }
        } 
        else if (command==0x11 || command==0x12)
        {
            unsigned char level = 255;
            json.addValue("insteon","event");
            json.addValue(mID,"id");
            json.addValue(this->getName(),"name");
            json.addValue("ack","trigger");
            json.addValue("switch","type");
            json.addValue(level,"value");
            this->setDeviceParameter(level);
        } 
        else if (command==0x13 || command==0x14) 
        {
            unsigned char level = 0;
            json.addValue("insteon","event");
            json.addValue(mID,"id");
            json.addValue(this->getName(),"name");
            json.addValue("ack","trigger");
            json.addValue("switch","type");
            json.addValue(level,"value");
            this->setDeviceParameter(level);
        }
    }
    else if (status==0b010)    // Group cleanup direct message
    {
        json.addValue("insteon","event");
        json.addValue(mID,"id");
        json.addValue(this->getName(),"name");
        json.addValue("unsolicited","trigger");

        unsigned char level = 0;
        if (cmd1==0x11 || cmd1==0x12){
            level = 255;
            json.addValue("switch","type");
            json.addValue(level,"value");
            this->setDeviceParameter(level);
        } else if (cmd1==0x13 || cmd1==0x14) {
            level = 0;
            json.addValue("switch","type");
            json.addValue(level,"value");
            this->setDeviceParameter(level);
        } else {
        }
     } else if (status ==0b110){
        if (cmd1==0x18)
        {
            /**
              * When someone dims a light manually, we get a frist event (cmd1=0x17) saying that a manual change
              * has started. cmd2 will be 1 if the user is brightening the light or 0 if dimming down.
              * When the desired brightness has been hit, we get a "manual change stopped" (cmd1=0x18).
              * We won't get any data informing us of the brightness selected. So we need to do a status query.
              */
            LOG("Manual change stopped for device "<<std::hex<<mID);
            // This is a "stop manual change" command. A dimmer has changed so we must query its new state
            this->writeCommand(mID,0x19,0);
        }
     }

}


void SwitchDevice::toJSON(Dumais::JSON::JSON& obj)
{
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(6) << mID;
        obj.addValue(ss.str(),"id");
        obj.addValue(this->getName(),"name");
        obj.addValue(this->getDeviceType(),"type");
        obj.addValue((unsigned int)this->getLevel(),"level");
}

