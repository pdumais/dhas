#ifndef SERVICE_H
#define SERVICE_H
#include <pthread.h>
#include "IEventProcessor.h"
#include "rest/RESTEngine.h"
#include "json/JSON.h"
#include <functional>
#include <future>


class Module{
private:
    pthread_t   mThreadHandle;
    volatile bool        mStopping;
    volatile bool        mSuspended;
    volatile bool        mNeedSuspend;
    std::promise<bool>   mStartedPromise;

protected:
    bool stopping();
    bool isSuspended();

    IEventProcessor *mpEventProcessor;

public:
	Module();
	~Module();

    void suspend();
    void resume();
    virtual void stop()=0;

    void startModule(IEventProcessor *p);
    void waitReady();
    void waitCompletion();
    virtual void run()=0;
    virtual std::string getName()=0;
    virtual void configure(Dumais::JSON::JSON& config)=0;

    virtual void registerCallBacks(RESTEngine*)=0;
    static std::vector<std::function<Module*()>> mModuleBuilders;
    virtual void appendPeriodicData(Dumais::JSON::JSON& data);

    void setStarted();
};


#endif

