#pragma once

#include <string>
#include <queue>
#include "ISound.h"
#include "ISoundDevice.h"
#include <list>
#include <thread>
#include "AlsaPCM.h"


#define SOUNDCARD_CHUNK_SIZE 160

namespace Dumais{
namespace Sound{

class SoundCard: public Dumais::Sound::ISoundDevice
{
private:
    std::string mDevice;
    int mChunkSize;
    unsigned char* mBuffer;
    AlsaPCM* mpAlsaPCM;

    void work();

protected:
    virtual unsigned int getSampleRate() { return mpAlsaPCM->getSampleRate(); }
    virtual unsigned int getSampleSize() { return mpAlsaPCM->getSampleSize(); } // 16bit stereo

public:
	SoundCard(const std::string& deviceName);
	~SoundCard();

    void start();
    void stop();

};

}
}


