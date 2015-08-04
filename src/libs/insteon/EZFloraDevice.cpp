#include "InsteonLogging.h"
#include "EZFloraDevice.h"
#include <iomanip>
#include <sstream>
#include <limits.h>

EZFloraDevice::EZFloraDevice(std::string name, InsteonID id, InsteonModem *p): InsteonDevice(name,id,p)
{
    mStatusByte = 0;
    mMeter = 0;

    for (int prog=0;prog<5;prog++) for (int valve=0;valve<8;valve++) mTimeouts[prog].valve[valve] = 0;
    
    // enable "get valve status change"
    // The CMD2 in the response message will contain the status byte. So we can use that information
    // to get the initial values. Killing two birds with one stone.
    writeCommand(id,0x44,0x09);


}

EZFloraDevice::~EZFloraDevice()
{
}

InsteonDeviceType EZFloraDevice::getDeviceType()
{
    return EZFlora;
}

void EZFloraDevice::toggle(unsigned char subdev)
{
    if (subdev <= 7)
    {
        
        if (getOpenedValve()==subdev)
        {
            turnOnOrOff(false,0,subdev);
        }
        else
        {
            turnOnOrOff(true,0,subdev);
        }
    } 
    else if (subdev <= 11)
    {
        if (getRunningProgram() == (subdev-8))
        {
            turnOnOrOff(false,0,subdev);
        }
        else 
        {
            turnOnOrOff(true,0,subdev);
        }
    }
}

void EZFloraDevice::turnOnOrOff(bool on, unsigned char level, unsigned char subdev)
{
    if (subdev<=7)
    {
        // a valve

        if (on)
        {
           // clear meter counter
           mpInsteonModem->writeCommand(this->mID,0x44,0x0F);
       
           // start valve
           mpInsteonModem->writeCommand(mID,0x40,subdev);
        }
        else
        {
            mpInsteonModem->writeCommand(mID,0x41,subdev);
        }

    } 
    else if (subdev <=11)
    {
        // a program

        if (on)
        {
           // clear meter counter
           mpInsteonModem->writeCommand(mID,0x44,0x0F);

           // start program
            mpInsteonModem->writeCommand(mID,0x42,subdev-8);
        }
        else
        {
            mpInsteonModem->writeCommand(mID,0x43,subdev-8);
        }

    }
}

void EZFloraDevice::processEvent(Dumais::JSON::JSON& json, unsigned char* buf)
{
    unsigned char flags = buf[8];
    unsigned char status = (flags & 0b11100000)>>5;
    unsigned char cmd1 = buf[9];
    unsigned char cmd2 = buf[10];

    if ((status==0b001 && (cmd1>=0x40 && cmd1<=0x44))||(status==0b100 && cmd1==0x27))
    {
        if (getLastDirectMessageSent().cmd1==0x40 && (getLastDirectMessageSent().extended))
        {
            // On init, we send 0x40 extended message to set programs. But we get a response back wich
            // looks exactly the same as the ACK for "turn on valve". So we check if last message
            // was extended to detect that.
            LOG("Ignoring EZFlora extended message reply");
            return; // Ignore the response
        }

        if (cmd1==0x27)
        {
            //TODO: get meter counter
        }

        unsigned char valveStatus = cmd2;
        bool runningProgram = valveStatus & 0b00100000;
        bool runningValve = valveStatus & 0b10000000;
        if (!runningProgram && !runningValve) // no valve open and no program running
        {
            if (cmd1!=0x44){
                // Water Off. meterCounter
                json.addValue("wateroff","event");
                json.addValue(mID,"id");
                json.addValue(this->getName(),"name");
                json.addValue(this->getDeviceType(),"type");
                json.addValue((unsigned int)getMeterCounter(),"meter"); 
            }

        } 
        else if (runningValve)
        {
            char previousValve = -1;
            unsigned char valve = valveStatus & 0b00000111; 
            if (valveStatus & 0b10000000)
            {
                if (valve != getOpenedValve())
                {
                    previousValve = getOpenedValve(); // another valve was opened before. It is now closed.
                }
            }
            if (cmd1!=0x44)
            {
                // A valve is opened. Implies that all other valves are closed since only one valve 
                // can be open at a time.
                // WaterOn event: valve, previous valve. previous meterCounter
                json.addValue("wateron","event");
                json.addValue(mID,"id");
                json.addValue(this->getName(),"name");
                json.addValue(this->getDeviceType(),"type");
                json.addValue((unsigned int)getMeterCounter(),"meter");
                json.addValue((int)previousValve,"previousvalve");
                json.addValue((int)valve,"valve");  
            }
        }

        mStatusByte = valveStatus;    
        LOG("EZFlora valvestatus = " << mStatusByte);
    }

}

void EZFloraDevice::toJSON(Dumais::JSON::JSON& obj)
{
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(6) << mID;
        obj.addValue(ss.str(),"id");
        obj.addValue(this->getName(),"name");
        obj.addValue(this->getDeviceType(),"type");
        obj.addValue((int)this->getOpenedValve(),"valve");
        obj.addValue((int)this->getRunningProgram(),"program");
        obj.addValue((unsigned int)this->getMeterCounter(),"meter");

}

int EZFloraDevice::getOpenedValve()
{
    if (mStatusByte & 0b10000000)
    {
        return (mStatusByte & 0b00000111);
    }
    return -1;
}

int EZFloraDevice::getRunningProgram()
{
    if (mStatusByte & 0b00100000)
    {
        return ((mStatusByte & 0b00011000)>>3);
    }
    return -1;
}

int EZFloraDevice::getMeterCounter()
{
    return mMeter;
}



