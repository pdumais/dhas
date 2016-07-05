#include "utils/Logging.h"
#include "SoundCard.h"
#include "SoundFile.h"
#include "SilenceSound.h"
#include <sstream>


using namespace Dumais::Sound;

SoundCard::SoundCard(const std::string& deviceName)
{
    this->mMusic = 0;
    this->mCurrentSound = 0;
    this->mpAlsaPCM = 0;
    this->mDevice = deviceName;
    this->mBuffer = 0;
}

SoundCard::~SoundCard(){
    this->stop();

    // Clear the queue
    ISound *s;
    while(this->mSoundQueue.get(s)) delete s;

    if (this->mpAlsaPCM) delete this->mpAlsaPCM;
    if (this->mBuffer) delete this->mBuffer;
}


void SoundCard::start()
{

    if (this->mCurrentSound) delete this->mCurrentSound;
    this->mCurrentSound = 0;
    this->mQueueWasEmpty = false;
    this->mInterruptCurrentSound=false;
    this->mpAlsaPCM = new AlsaPCM(mDevice.c_str());

    // We can write the size of 1 period at each interrupt
    this->mChunkSize = this->mpAlsaPCM->getPeriodSizeInSamplesCount()*this->mpAlsaPCM->getSampleSize();
    if (this->mChunkSize == 0) return;

    this->mBuffer = new unsigned char[this->mChunkSize];

    this->mStopping = false;
    this->mThread = std::thread([this](){
        while (!mStopping)
        {
            this->work();
        }
    });

}

void SoundCard::stop()
{
    if (this->mThread.joinable())
    {
        this->mStopping = true;
        this->setWorking();
        this->mThread.join();
    }

    delete this->mpAlsaPCM;
    this->mpAlsaPCM = 0;
}

void SoundCard::work()
{
    if (!mCurrentSound)
    {
        if (this->mSoundQueue.get(mCurrentSound))
        {
            mQueueWasEmpty=false;
            this->mInterruptCurrentSound=false;
            LOG("SoundCard: Will play " << mCurrentSound->toString());
        } else {
            if (!mQueueWasEmpty) // in order to notify once only.
            {
                this->notifyQueueEmpty();
                mQueueWasEmpty=true;
            }
        }
    }

    if (!this->mMusic && !this->mCurrentSound)
    {
        this->setIdle();
        return;
    }

    int soundindex = 0;
    if (this->mMusic)
    {
        this->mMusic->getSample(this->mChunkSize,(char*)&mBuffer[0]);
    }

    int sampleSize = this->getSampleSize();
    int numberOfFramesToSend = this->mChunkSize;
    if (sampleSize >0)
    {
        numberOfFramesToSend = numberOfFramesToSend/sampleSize;
    }
    if (mCurrentSound)   // if there is a sound to be played from queue, hide the music
    {
        // overwrite music with sound
        soundindex=mCurrentSound->getSample(this->mChunkSize,(char*)&mBuffer[0]);

        if (soundindex<this->mChunkSize || this->mInterruptCurrentSound)
        {
            delete mCurrentSound;
            mCurrentSound = 0;

            if (this->mMusic)
            {
                //Note: if buffer wasn't filled completely, it contains music anyways.
            }
            else
            {
                numberOfFramesToSend = soundindex / this->getSampleSize();
            }
        }
    }

    //TODO DUMAIS: soundcard must be non-blocking and we should retry to send the sample if soundcard buffer was full
    this->mpAlsaPCM->playChunk((char*)&mBuffer[0], numberOfFramesToSend);
}

