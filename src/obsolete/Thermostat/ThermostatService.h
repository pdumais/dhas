#ifndef THERMOSTATSERVICE_H
#define THERMOSTATSERVICE_H
#include "Service.h"

class ThermostatService: public Service{
private:
    volatile time_t mLastPollTime;
    time_t mLastProgramPollTime;
    std::string mServer;
    Dumais::JSON::JSON mStatJson;
    Dumais::JSON::JSON mProgramJson;

public:
	ThermostatService();
	~ThermostatService();

    virtual void stop();
    virtual void run();
    virtual std::string getName(){return "Thermostat";}

    void setIP_callback(Dumais::JSON::JSON& json, RESTParameters* params);    
    void setTemperature_callback(Dumais::JSON::JSON& json, RESTParameters* params);
    void getStats_callback(Dumais::JSON::JSON& json, RESTParameters* params);
    void setMode_callback(Dumais::JSON::JSON& json, RESTParameters* params);

    void registerCallBacks(RESTEngine* pEngine);

};

#endif

