#pragma once
#include "Module.h"
#include <queue>
#include <string>
#include "ISerialPort.h"

/*
 Sounnd names provided must be names only. This object will append ".wav" to it and will prefix it with "sounds/"
*/

class AlarmModule: public Module
{
private:
    ISerialPort* mpSerialPort;
    
protected:
    virtual void configure(Dumais::JSON::JSON& config);

public:
	AlarmModule();
	~AlarmModule();

    void registerCallBacks(ThreadSafeRestEngine* pEngine);    

    virtual void run();
    virtual std::string getName(){return "alarm";}
    virtual void stop();
};


