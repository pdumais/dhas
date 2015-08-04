#pragma once
#include "ISoundPlaylistObserver.h"
#include "SoundMPSCRingBuffer.h"
#include <list>
#include "ISound.h"
#include <thread>
#include <condition_variable>

namespace Dumais{
namespace Sound{

class ISoundPlaylistObserver;

class ISoundDevice
{
protected:
    Dumais::Sound::ISound* mMusic;
    Dumais::Sound::MPSCRingBuffer<Dumais::Sound::ISound*> mSoundQueue;
    std::list<ISoundPlaylistObserver*> mObservers;
    Dumais::Sound::ISound *mCurrentSound;
    std::mutex mWaitLock;
    std::condition_variable_any mWaitCondition;
    std::thread mThread;
    bool mStopping;
    bool mInterruptCurrentSound;
    bool mQueueWasEmpty;
    void notifyQueueEmpty();

    virtual unsigned int getSampleRate() = 0;
    virtual unsigned int getSampleSize() = 0;

public:
    virtual void work() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    void setMusic(const char *filename, SoundFormat format);
    void play(const char *filename, SoundFormat format);
    void silence(int seconds);
    void addObserver(ISoundPlaylistObserver* obs);
    void clearQueue();
    void setIdle();
    void setWorking();
};

}
}
