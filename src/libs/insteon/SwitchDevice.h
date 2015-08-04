#ifndef ONOFFDEVICE_H
#define ONOFFDEVICE_H

#include "InsteonDevice.h"

class SwitchDevice: public InsteonDevice{
private:

public:
	SwitchDevice(std::string name, InsteonID id, InsteonModem *p);
	~SwitchDevice();

    unsigned char getLevel();

    virtual void turnOnOrOff(bool on, unsigned char level, unsigned char subdev);
    virtual void toggle(unsigned char subdev);

    virtual InsteonDeviceType getDeviceType();
    virtual void processEvent(Dumais::JSON::JSON& json,unsigned char* buf);
    virtual void toJSON(Dumais::JSON::JSON& json);
};

#endif

