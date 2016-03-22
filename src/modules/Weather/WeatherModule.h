#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H
#include "Module.h"

enum HeatMode
{
    Off,
    Heat,
    Cool
};

struct ThermostatInfo
{
    std::string temperatures[8];
    std::string setpoint;
    bool heat;
    bool cool;
    bool fan;
    bool schedule;
    std::string filter;
    HeatMode mode;
};

class WeatherModule: public Module{
private:
    ThermostatInfo parseState(const std::string& str);
    std::string getXMLValue(const std::string& subject, const std::string& str, int matchindex);
    void convertInfoToJSON(Dumais::JSON::JSON& j, const ThermostatInfo& info);
    volatile time_t mLastPollTime;
    std::string mServer;
    std::string mTemperatures[4];
    HeatMode mRunStatus;

protected:
    virtual void configure(Dumais::JSON::JSON& config);
public:
	WeatherModule();
	~WeatherModule();

    virtual void stop();
    virtual void run();
    virtual std::string getName(){return "Weather";}

    virtual void appendPeriodicData(Dumais::JSON::JSON& data);

    std::string getTemperature(int id);
    void setIP_callback(RESTContext* context);    
    void getStats_callback(RESTContext* context);
    void getThermostat_callback(RESTContext* context);
    void resetfilter_callback(RESTContext* context);
    void setpoint_callback(RESTContext* context);
    void setmode_callback(RESTContext* context);
    void toggleschedule_callback(RESTContext* context);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
};

#endif

