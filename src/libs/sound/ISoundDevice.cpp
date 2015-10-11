#include "utils/Logging.h"
#include "ISoundDevice.h"
#include "SilenceSound.h"
#include "SoundFile.h"

using namespace Dumais::Sound;

void ISoundDevice::notifyQueueEmpty()
{
    for (auto it = this->mObservers.begin();it!=this->mObservers.end();it++)
    {
        (*it)->onSoundQueueEmpty();
    }
}

void ISoundDevice::silence(int seconds)
{
    LOG("Adding silence of " << seconds << " seconds in sound device queue");
    SilenceSound *sound = new SilenceSound(seconds, this->getSampleRate(), this->getSampleSize());
    this->mSoundQueue.put(sound);
    this->setWorking();
}

void ISoundDevice::play(const char *filename, Dumais::Sound::SoundFormat format)
{
    LOG("Adding " << filename << " in sound device queue");
    SoundFile *file = new SoundFile();
    if (file->open(filename,format))
    {
        this->mSoundQueue.put(file);
        this->setWorking();
    } else {
        delete file;
    }
}

void ISoundDevice::setMusic(const char *filename, SoundFormat format)
{
    if (this->mMusic) delete this->mMusic;
    this->mMusic = new SoundFile(true);
    ((SoundFile*)(this->mMusic))->open(filename,format);
    this->setWorking();
}

void ISoundDevice::clearQueue()
{
    ISound *s;
    while (this->mSoundQueue.get(s)) delete s;
    this->mInterruptCurrentSound = true;
}

void ISoundDevice::addObserver(ISoundPlaylistObserver* obs)
{
    this->mObservers.push_back(obs);
}

void ISoundDevice::setIdle()
{
    this->mWaitLock.lock();
    this->mWaitCondition.wait(mWaitLock);
    this->mWaitLock.unlock();
}

void ISoundDevice::setWorking()
{
    this->mWaitCondition.notify_all();
}
