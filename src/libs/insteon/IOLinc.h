#ifndef IOLINCDEVICE_H
#define IOLINCDEVICE_H

#include "InsteonDevice.h"

class IOLinc: public InsteonDevice{
private:

public:
	IOLinc(std::string name, InsteonID id, InsteonModem *p);
	~IOLinc();

    bool getState();

    virtual void turnOnOrOff(bool on, unsigned char level, unsigned char subdev);
    virtual void toggle(unsigned char subdev);

    virtual InsteonDeviceType getDeviceType();
    virtual void processEvent(Dumais::JSON::JSON& json,unsigned char* buf);
    virtual void toJSON(Dumais::JSON::JSON& json);
};

#endif

