#ifndef KEYPADLINCDEVICE_H
#define KEYPADLINCDEVICE_H

#include "InsteonDevice.h"

class KeypadLinc: public InsteonDevice{
private:
//    unsigned char mButtonGroups[8];
public:
	KeypadLinc(std::string name, InsteonID id, InsteonModem *p);
	~KeypadLinc();

    virtual void turnOnOrOff(bool on, unsigned char level, unsigned char subdev);
    virtual void setGroup(unsigned char group, unsigned char data1,unsigned char data2,unsigned char data3);
    virtual InsteonDeviceType getDeviceType();
    virtual void processEvent(Dumais::JSON::JSON& json,unsigned char* buf);
    virtual void toJSON(Dumais::JSON::JSON& json);
};

#endif

