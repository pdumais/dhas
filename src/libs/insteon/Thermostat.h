#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include "InsteonDevice.h"
#include "ThermostatProgram.h"

class Thermostat: public InsteonDevice{
public:
    enum ThermostatMode
    {
        Off,
        Heat,
        Cool,
        Auto,
        ManualFan
    };

private:
    char mCoolPoint;
    char mHeatPoint;
    char mHumidity;
    char mTemperature;
    bool mProgramActive;
    ThermostatMode mOperatingMode;
    ThermostatMode mOperatingStatus;
    ThermostatProgram mThermostatProgram;
    unsigned char mLastHourCheck;

public:
	Thermostat(std::string name, InsteonID id, InsteonModem *p);
	~Thermostat();

    void setHeatPoint(char v);
    void setCoolPoint(char v);
    void setOperatingMode(ThermostatMode mode);
    void setProgramPoint(unsigned char t, ThermostatMode mode, unsigned char hour, unsigned char weekday);
    void getProgramPoints(Dumais::JSON::JSON& json);
    void removeProgramPoint(unsigned char hour, unsigned char weekday);

    char getCoolSetPoint();
    char getHeatSetPoint();
    float getTemperature();
    char getHumidity();
    ThermostatMode getOperatingMode();
    ThermostatMode getOperatingState();

    void checkThermostatProgam(unsigned char t);
    void activateProgram(bool active);

    virtual void onTimer(time_t t);
    virtual InsteonDeviceType getDeviceType();
    virtual void processEvent(Dumais::JSON::JSON& json,unsigned char* buf);
    virtual void toJSON(Dumais::JSON::JSON& json);
};

#endif

