#include "utils/Logging.h"
#include "IOLinc.h"
#include <iomanip>
IOLinc::IOLinc(std::string name, InsteonID id, InsteonModem *p): InsteonDevice(name,id,p)
{
    mDeviceParameter = 0;

    // Query that device's initial status
    // when param2==0, it is for status of relay. When param2==1, it is for status of sensor
    this->writeCommand(id,0x19,1);
}

IOLinc::~IOLinc(){
}

bool IOLinc::getState()
{
    return mDeviceParameter==0;
}

InsteonDeviceType IOLinc::getDeviceType()
{
    return IOLincDevice;
}


void IOLinc::turnOnOrOff(bool on, unsigned char level, unsigned char subdev)
{
    // Note: 
    // I could never make the on/off work. So I am only sending On, which seems to trigger the relay
    // regardless of the sensor status. This is because of the way we send the command. By
    // sending the direct message, the IOLinc only listens for "on". So we need to implement
    // the logic ourself to know if door is open or closed.
    // It would be possible that we call this 2 times in a row very fast, but since the relay
    // is momentary and stays on for 2 sec, we know the command will be ignored. So this gives us
    // 2 seconds for the sensor to change state

    // Only open if it is closed, and close if it is opened (this would be WantOn XOR IsOn)
    // If we called this function to close the door, it needs to be open and vice-versa. 
    // Either way, we always send the "on" command. But we wont send anything if we try to open
    // the door while it is already open.
    if (getState() ^ on)
    {
        this->writeCommand(mID,0x11,255);
    }
}

void IOLinc::toggle(unsigned char subdev)
{
    this->writeCommand(mID,0x11,255);
}



void IOLinc::processEvent(Dumais::JSON::JSON& json, unsigned char* buf)
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
            this->setDeviceParameter((cmd2==0)?0:255);
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
                LOG("Initial level for device " <<std::hex<<mID << " is " << cmd2);
            }
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
            json.addValue("iolinc","type");
            json.addValue(level,"value");
            this->setDeviceParameter(level);
        } else if (cmd1==0x13 || cmd1==0x14) {
            level = 0;
            json.addValue("iolinc","type");
            json.addValue(level,"value");
            this->setDeviceParameter(level);
        }
     }

}


void IOLinc::toJSON(Dumais::JSON::JSON& obj)
{
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(6) << mID;
        obj.addValue(ss.str(),"id");
        obj.addValue(this->getName(),"name");
        obj.addValue(this->getDeviceType(),"type");
        obj.addValue(this->getState()?255:0,"level");
}

