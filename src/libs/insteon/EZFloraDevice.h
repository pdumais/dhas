#ifndef EZFLORADEVICE_H
#define EZFLORADEVICE_H

#include "InsteonDevice.h"

struct ValveTimeout
{
    unsigned char valve[8];
};


class EZFloraDevice : public InsteonDevice{
private:
    unsigned char mStatusByte;
    unsigned int mMeter;
    ValveTimeout mTimeouts[5]; // 0 is timeout, 1,2,3,4 are programs

public:
	EZFloraDevice(std::string name, InsteonID id, InsteonModem *p);
	~EZFloraDevice();

    virtual void turnOnOrOff(bool on, unsigned char level, unsigned char subdev);
    virtual void toggle(unsigned char subdev);
    virtual InsteonDeviceType getDeviceType();
    virtual void processEvent(Dumais::JSON::JSON& json, unsigned char* buf);
    virtual void toJSON(Dumais::JSON::JSON& json);

    int getOpenedValve();
    int getRunningProgram();
    int getMeterCounter();
};

#endif

