#include "DHASLogging.h"
#include "Module.h"

#include "ModuleProvider.h"
#include <unistd.h>

std::vector<std::function<Module*()>> Module::mModuleBuilders;

void *ThreadStarter(void *p)
{
    Module* pModule = (Module*)p;
    pModule->run();
}

Module::Module(){
    mNeedSuspend = false;
    mSuspended = true;
}

Module::~Module()
{
}

void Module::startModule(IEventProcessor *p)
{
    mpEventProcessor = p;
    mStopping = false;
    mSuspended = false;
    pthread_create(&mThreadHandle, 0, ThreadStarter, this);
}

void Module::setStarted()
{
    mStartedPromise.set_value(true);
}

void Module::waitReady()
{
    mStartedPromise.get_future().wait();
}

void Module::suspend()
{
    mNeedSuspend = true;
    while (!mSuspended);  // wait for the thread to be suspended
}

void Module::resume()
{
    mNeedSuspend = false;
}


void Module::waitCompletion()
{
    mStopping = true;
    pthread_join(mThreadHandle,0);
}

bool Module::isSuspended()
{
    return mSuspended;
}

bool Module::stopping()
{
    // Every thread calls this function in their while loop. So if we need to suspend, keep waiting in here
    while (mNeedSuspend)
    {
        mSuspended = true;
        usleep(200000);
    }
    mSuspended = false;

    return mStopping;
}

/*bool Module::runCommand(Dumais::JSON::JSON& retJSON, std::string cmd, Dumais::JSON::JSON& params)
{
    if (mCallbackList.find(cmd)==mCallbackList.end()) return false;

    mCallbackList[cmd]->call(retJSON,params);
    return true;
}

void Module::addCallBack(std::string name, CallBack* p)
{
    mCallbackList[name] = p;
}*/

void Module::appendPeriodicData(Dumais::JSON::JSON& data)
{
}
