#include "ThreadPool.h"
#include <unistd.h>

void *threadStarter(void *p)
{
    ThreadPool *tp = (ThreadPool*)p;
    tp->threadFunction();
}

ThreadPool::ThreadPool(unsigned int threadCount)
{
    mThreadCount = threadCount;
    mStop = true;

    pthread_cond_init(&mWaitCondition,0);
    pthread_mutex_init(&mWaitConditionLock,0);
    pthread_mutex_init(&mQueueLock,0);
    pthread_mutex_init(&mCountLock,0);
    
    initPool();
}


ThreadPool::~ThreadPool()
{
    delete[] mpThreads;
    pthread_cond_destroy(&mWaitCondition);
    pthread_mutex_destroy(&mWaitConditionLock);
    pthread_mutex_destroy(&mQueueLock);
    pthread_mutex_destroy(&mCountLock);

}

void ThreadPool::initPool()
{
    mRunningThreads = 0;
    mpThreads = new pthread_t[mThreadCount];
    mStop = false;
    for (unsigned int i=0;i<mThreadCount;i++)
    {
        pthread_create(&mpThreads[i], 0, threadStarter, this);
    }

    // wait for all threads to be running
    while (mRunningThreads != mThreadCount) usleep(100000);


}

void ThreadPool::threadFunction()
{
    pthread_mutex_lock(&mCountLock);
    mRunningThreads++;
    pthread_mutex_unlock(&mCountLock);

    while (!mStop)
    {
        pthread_mutex_lock(&mQueueLock);
        IThreadCallBack* pJob = 0;
        if (mJobs.size()>0)
        {
            pJob = mJobs.front();
            mJobs.pop();
        }
        pthread_mutex_unlock(&mQueueLock);

        if (pJob)
        {
            pJob->run(); 
            delete pJob;
        }
        else 
        {
            pthread_mutex_lock(&mWaitConditionLock);
            pthread_cond_wait(&mWaitCondition, &mWaitConditionLock);
            pthread_mutex_unlock(&mWaitConditionLock);
        }
    }
}

void ThreadPool::createJob(IThreadCallBack* pCallBack)
{
   pthread_mutex_lock(&mQueueLock);
   mJobs.push(pCallBack);
   pthread_mutex_unlock(&mQueueLock);

   // wakeup a thread so that it can pickup the job
   pthread_mutex_lock(&mWaitConditionLock);
   pthread_cond_signal(&mWaitCondition);
   pthread_mutex_unlock(&mWaitConditionLock);
}

void ThreadPool::stopPool()
{
    mStop = true;
    pthread_mutex_lock(&mWaitConditionLock);
    pthread_cond_broadcast(&mWaitCondition);
    pthread_mutex_unlock(&mWaitConditionLock);

    for (unsigned int i=0;i<mThreadCount;i++)
    {
        pthread_join(mpThreads[i],0);
    }
}
