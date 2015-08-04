#ifndef INSTEONDEVICE_H
#define INSTEONDEVICE_H
#include "JSON.h"
#include <string>
#include "InsteonModem.h"

enum InsteonDeviceType
{
    OnOff,
    EZFlora,
    ThermostatDevice,
    IOLincDevice,
    KeypadLincDevice
};

enum TransmissionStatus
{
    Idle,
    WaitingAckOfDirectMessage
};

class InsteonDevice{
private:
    time_t mLastMessageTimeStamp;
protected:
    std::string mName;
    InsteonID mID;
    InsteonDirectMessage mLastDirectMessageSent;
    bool mInitialized;
    InsteonDeviceType mType;
    unsigned char mDeviceParameter;
    InsteonModem *mpInsteonModem; 
    TransmissionStatus mTransmissionStatus;    

    void writeCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2);
    void writeGroupCleanupCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2);
    void writeI2CSCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char *buf=0);
    void writeExtendedCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char* data);

public:
	InsteonDevice(std::string name, InsteonID id, InsteonModem* p);
	virtual ~InsteonDevice();

    std::string getName();
    InsteonID getID();

    InsteonDirectMessage getLastDirectMessageSent();
    void setLastDirectMessageSent(InsteonDirectMessage cmd);
    bool isIdle();

    void setDeviceParameter(unsigned char param);

    virtual InsteonDeviceType getDeviceType()=0;
    virtual void setGroup(unsigned char group, unsigned char data1,unsigned char data2,unsigned char data3);
    virtual void onTimer(time_t t);
    
    bool isInitialized();
    void setInitialized();

    void onInsteonMessage(Dumais::JSON::JSON& json, unsigned char* buf);

    virtual void processEvent(Dumais::JSON::JSON& json, unsigned char* buf)=0;
    virtual void toJSON(Dumais::JSON::JSON& json)=0;

    virtual void toggle(unsigned char subdev) {}
    virtual void turnOnOrOff(bool on, unsigned char level, unsigned char subdev) {}

};

#endif

