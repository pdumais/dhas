#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H
#include "Module.h"

class WeatherModule: public Module{
private:
    volatile time_t mLastPollTime;
    std::string mServer;
    std::string mTemperatures[4];

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
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
};

#endif

