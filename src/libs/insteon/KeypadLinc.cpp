#include "InsteonLogging.h"
#include "KeypadLinc.h"
#include <iomanip>
KeypadLinc::KeypadLinc(std::string name, InsteonID id, InsteonModem *p): InsteonDevice(name,id,p)
{
    mDeviceParameter = 0;
    //for (int i=0;i<8;i++) mButtonGroups[i]=0;
}

KeypadLinc::~KeypadLinc(){
}

InsteonDeviceType KeypadLinc::getDeviceType()
{
    return KeypadLincDevice;
}


void KeypadLinc::processEvent(Dumais::JSON::JSON& json, unsigned char* buf)
{
    // Get the status
    unsigned char flags = buf[8];
    unsigned char status = (flags & 0b11100000)>>5;
    unsigned char cmd1 = buf[9];
    unsigned char cmd2 = buf[10];
    bool ackOfDirectMessage = (status==0b001);

    if (status==0b010)    // Group cleanup direct message
    {
        json.addValue("insteon","event");
        json.addValue(mID,"id");
        json.addValue(this->getName(),"name");
        json.addValue("unsolicited","trigger");

        unsigned char level = 0;
        if (cmd1==0x11){
            level = 255;
            json.addValue("keypad","type");
            json.addValue(cmd2,"button");
            json.addValue(level,"value");

            /* we turn button off again so that it gets updated by script instead. This is a hack
             * otherwise the button is on, but if nothing reacts to it, it will stay on
             */
            //turnOnOrOff(false,0,cmd2);
        } else if (cmd1==0x13) {
            level = 0;
            json.addValue("keypad","type");
            json.addValue(cmd2,"button");
            json.addValue(level,"value");

            /* we turn button on again so that it gets updated by script instead. This is a hack
             * otherwise the button is on, but if nothing reacts to it, it will stay on
             */
            //turnOnOrOff(true,0,cmd2);
        }
     }

}

void KeypadLinc::setGroup(unsigned char group, unsigned char data1,unsigned char data2,unsigned char data3)
{
    /*unsigned char button = group; //TODO: temporary, should be data3
    if (button > 7) return;
    mButtonGroups[button] = group;
    Logging::log("Keypadlinc button %i on group %i",button,group);*/
}

void KeypadLinc::turnOnOrOff(bool on, unsigned char level, unsigned char button)
{
    if (button>=8) return;

    // that will only work if, when linking, group1 was used with btn 1, grp 2 with btn 2 etc...
    writeGroupCleanupCommand(mID,on?0x11:0x13,button);
}

void KeypadLinc::toJSON(Dumais::JSON::JSON& obj)
{
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(6) << mID;
        obj.addValue(ss.str(),"id");
        obj.addValue(this->getName(),"name");
        obj.addValue(this->getDeviceType(),"type");
//        obj.addValue(this->getState()?255:0,"level");
}
