#pragma once
#include <queue>
#include <pthread.h>

class IThreadCallBack
{
public:
   virtual void run() = 0;
};

template <class T,class T2>
class ThreadCallBack: public IThreadCallBack
{
private:
    void (T::*mpCallback)(T2 param);
    T *mpObject;
    T2 mParam;
public:
    ThreadCallBack(T *obj, void (T::*pCB)(T2 param), T2 param)
    {
        mParam = param;
        mpObject = obj;
        mpCallback = pCB;
    }

    ~ThreadCallBack(){};

    void run()
    {
        (mpObject->*mpCallback)(mParam);
    }
};

class ThreadPool
{
private:
    unsigned int mThreadCount;
    std::queue<IThreadCallBack*> mJobs;
    bool mStop;
    pthread_cond_t mWaitCondition;
    pthread_mutex_t mWaitConditionLock;
    pthread_mutex_t mQueueLock;
    pthread_mutex_t mCountLock;
    pthread_t* mpThreads;
    volatile int mRunningThreads;
    void initPool();

public:
   ThreadPool(unsigned int threadCount);
   virtual ~ThreadPool();
   void threadFunction();

   void stopPool();
   void createJob(IThreadCallBack* pCallBack);
};


