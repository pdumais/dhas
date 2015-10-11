#include "utils/Logging.h"
#include "Thermostat.h"
#include <iomanip>

/*
the thermostat needs to be a controller of group 0xEF for the PLM.
    to do this, I had to send cmd1=0x09,cmd2=0xEF to the thermostat then
    press 3sec on the PLM set button
*/

Thermostat::Thermostat(std::string name, InsteonID id, InsteonModem *p): InsteonDevice(name,id,p)
{
    mCoolPoint = 0;
    mHeatPoint = 0;
    mHumidity = 0;
    mTemperature = 0;
    mOperatingMode = Off;
    mOperatingStatus = Off;
    mLastHourCheck = 255;

    // Set Celcius mode
    this->writeI2CSCommand(id,0x6B,0x11);
    // Request operating mode
    this->writeI2CSCommand(id,0x6B,0x02);
    // Request current temperature
    this->writeI2CSCommand(id,0x6B,0x03);
    // Enable receiving status changed events
    this->writeI2CSCommand(id,0x6B,0x16);

    // request current humidity level: http://board.homeseer.com/showpost.php?p=1045535&postcount=4
    this->writeI2CSCommand(id,0x6A,0x60);

    // request setpoint
    // This will only return heat or cool depending on mode (and heat only if Auto). So this is limited
    this->writeI2CSCommand(id,0x6A,0x20);
}

Thermostat::~Thermostat()
{
}

char Thermostat::getCoolSetPoint()
{
    return mCoolPoint/2;
}

char Thermostat::getHeatSetPoint()
{
    return mHeatPoint/2;
}

float Thermostat::getTemperature()
{
    return (float)mTemperature/2.0;
}

char Thermostat::getHumidity()
{
    return mHumidity;
}

Thermostat::ThermostatMode Thermostat::getOperatingMode()
{
    return mOperatingMode;
}

Thermostat::ThermostatMode Thermostat::getOperatingState()
{
    return mOperatingStatus;
}


void Thermostat::setCoolPoint(char v)
{
    this->writeI2CSCommand(mID,0x6C,v*2);
}

void Thermostat::setHeatPoint(char v)
{
    this->writeI2CSCommand(mID,0x6D,v*2);
}


void Thermostat::setOperatingMode(ThermostatMode mode)
{
    unsigned char cmd2 = 0x09;
    switch (mode)
    {
        case Off:
            cmd2 = 0x09;
            break;
        case Heat:
            cmd2 = 0x04;
            break;
        case Cool:
            cmd2 = 0x05;
            break;
        case Auto:
            cmd2 = 0x06;
            break;
        case ManualFan: 
            cmd2 = 0x07;
            break;
    }
    this->writeI2CSCommand(mID,0x6B,cmd2);

}

InsteonDeviceType Thermostat::getDeviceType()
{
    return ThermostatDevice;
}



//http://forum.universal-devices.com/viewtopic.php?f=27&t=8745&start=30
void Thermostat::processEvent(Dumais::JSON::JSON& json, unsigned char* buf)
{
    // Get the status
    unsigned char flags = buf[8];
    unsigned char status = (flags & 0b11100000)>>5;
    unsigned char cmd1 = buf[9];
    unsigned char cmd2 = buf[10];
    bool ackOfDirectMessage = (status==0b001);

    if (status==0)
    {
        if (cmd1==0x6e)
        {
            mTemperature = cmd2;
        }
        else if (cmd1==0x6f)
        {
            mHumidity = cmd2;
        }
        // This is really buggy. Sometimes, I get operating status events with message 0x70.
        // and sometimes I get them with 0x6B
        else if (cmd1==0x70)
        {
            if (cmd2==0)
            {
                mOperatingMode = Off;
            }
            else if (cmd2==1)
            {
                mOperatingMode = Heat;
            }
            else if (cmd2==2)
            {
                mOperatingMode = Cool;
            }
        }
        else if (cmd1==0x71)
        {
            mCoolPoint = cmd2*2;
        }
        else if (cmd1==0x72)
        {
            mHeatPoint = cmd2*2;
        }

    }

    /*if (status==0b110)
    {
        if (cmd1==0x11)
        {
            if (buf[7]==1)
            {
                mOperatingStatus = Cool;
            }
            else if (buf[7]==2)
            {
                mOperatingStatus = Heat;
            }
        } 
        else if (cmd1==0x13)
        {
            mOperatingStatus = Off;
        }
    } 
    else*/  if (ackOfDirectMessage)
    {
        /*if (cmd1==0x6C && getLastDirectMessageSent().cmd1==0x6C)
        {
            mCoolPoint = cmd2;
        }
        if (cmd1==0x6D && getLastDirectMessageSent().cmd1==0x6D)
        {
            mHeatPoint = cmd2;
        }*/

        if (cmd1==0x6B && getLastDirectMessageSent().cmd1==0x6B)
        {
            switch (getLastDirectMessageSent().cmd2)
            {
                case 0x02:
                {
                    mOperatingMode = (ThermostatMode)cmd2;
                }
                break;
                case 0x03:
                {
                    mTemperature = cmd2;
                }
                break;
                case 0x0D:
                {
                //THIS NOT TRUE
                    if (cmd2&1) mOperatingStatus = Cool;
                    if (cmd2&2) mOperatingStatus = Heat;
                }
                break;
                /*case 0x04:
                {
                    mOperatingMode = Heat;
                }
                break;
                break;
                case 0x05:
                {
                    mOperatingMode = Cool;
                }
                break;
                break;
                case 0x06:
                {
                    mOperatingMode = Auto;
                }
                break;
                break;
                case 0x07:
                {
                    mOperatingMode = ManualFan;
                }
                break;
                break;
                case 0x08:
                {
                }
                break;
                break;
                case 0x09:
                {
                    mOperatingMode = Off;
                }
                break;*/

            }

        } 
        else if (cmd1==0x6A && getLastDirectMessageSent().cmd1==0x6A)
        {
            switch (getLastDirectMessageSent().cmd2)
            {
                case 0x60:
                {
                    mHumidity = cmd2;
                }
                break;
                case 0x20:
                {
                    if (mOperatingMode==Auto || mOperatingMode==Heat)
                    {
                        mHeatPoint = cmd2;
                    }
                    else if (mOperatingMode==Cool)
                    {
                        mCoolPoint = cmd2;
                    }
                }
            }
        }
    }
}


void Thermostat::setProgramPoint(unsigned char t, ThermostatMode mode, unsigned char hour, unsigned char weekday)
{
    unsigned char absoluteHour = (weekday-1)*24 + hour;
    mThermostatProgram.setPoint(absoluteHour,t,mode);
}

void Thermostat::removeProgramPoint(unsigned char hour, unsigned char weekday)
{
    unsigned char absoluteHour = (weekday-1)*24 + hour;
    mThermostatProgram.removePoint(absoluteHour);

}

void Thermostat::getProgramPoints(Dumais::JSON::JSON& json)
{
    json.addList("points");

    for (unsigned char i=0;i<(24*7);i++)
    {
        ProgramPoint point = mThermostatProgram.getPoint(i);
        if (point.mode > 0)
        {
            Dumais::JSON::JSON& obj = json["points"].addObject("ProgramPoint");
            obj.addValue(point.temp,"temperature");
            obj.addValue(point.mode,"mode");      
            obj.addValue(i,"hour");
        }
    }
}

void Thermostat::onTimer(time_t t)
{
    tm* ti;
    ti = localtime(&t);
    unsigned char h = (ti->tm_wday*24)+ti->tm_hour;
    checkThermostatProgam(h);
}

void Thermostat::checkThermostatProgam(unsigned char hour)
{
    if (hour==mLastHourCheck) return;
    mLastHourCheck = hour;

    if (!mThermostatProgram.isActive()) return;
    ProgramPoint point = mThermostatProgram.getPoint(hour);
    if (point.mode > 0)
    {
        LOG("Checking thermostat program point for hour "<<hour<<". Found t="<<point.temp<<", mode="<<point.mode);
        if (point.mode == 1)
        {
            setHeatPoint(point.temp);
        }
        else if (point.mode ==2)
        {
            setCoolPoint(point.temp);
        }
    }
    else
    {
        LOG("Checking thermostat program point for hour "<<hour<<". No points found");
    }

}

void Thermostat::activateProgram(bool active)
{
    mThermostatProgram.setActive(active);
}

void Thermostat::toJSON(Dumais::JSON::JSON& obj)
{
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(6) << mID;
        obj.addValue(ss.str(),"id");
        obj.addValue(this->getName(),"name");
        obj.addValue(this->getDeviceType(),"type");
        obj.addValue((unsigned int)this->getTemperature(),"temperature");
        obj.addValue((unsigned int)this->getHeatSetPoint(),"heatpoint");
        obj.addValue((unsigned int)this->getCoolSetPoint(),"coolpoint");
        obj.addValue((unsigned int)this->getHumidity(),"humidity");
        obj.addValue((unsigned int)this->getOperatingMode(),"operatingmode");
        obj.addValue((unsigned int)this->getOperatingState(),"operatingstate");
        obj.addValue((unsigned int)mThermostatProgram.isActive(),"programactive");
}


