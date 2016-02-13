#ifndef IOSERVICE_H
#define IOSERVICE_H
#include "Module.h"
#include "SerialPort.h"
#include <mutex>
#include <map>

struct WebRelay
{
    std::string ip;
    std::string id;
};

class IOModule: public Module{
private:
    volatile time_t mLastPollTime;
    std::string mServer;
    std::map<std::string,WebRelay> mWebRelayList;
    SerialPort *mpSerialPort;
    unsigned char mCurrentInputStatus;
    std::mutex mSerialPortLock;    

    void statusToJSON(Dumais::JSON::JSON &json);

protected:
    virtual void configure(Dumais::JSON::JSON& config);

public:
	IOModule();
	~IOModule();

    virtual void stop();
    virtual void run();
    virtual std::string getName(){return "IO";}

    void getStatus_callback(RESTContext* context);    
    void addWebRelay_callback(RESTContext* context);    
    void triggerRelay_callback(RESTContext* context);    
    void triggerIORelay_callback(RESTContext* context);    
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
};

#endif

