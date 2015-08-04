#ifndef SOUNDDEVICE_H
#define SOUNDDEVICE_H
#include <alsa/asoundlib.h>

namespace Dumais{
namespace Sound{

class AlsaPCM{
private:
    snd_pcm_t   *mpDeviceHandle;
    snd_pcm_uframes_t mBufferSize;
    snd_pcm_uframes_t mPeriodSize;
    unsigned int mSampleRate;
    int     mFrameSize;
public:
	AlsaPCM(const char* device);
	~AlsaPCM();
    void playChunk(char *buf, size_t numFrames);

    int getBufferSizeInSamplesCount() { return mBufferSize; }
    int getPeriodSizeInSamplesCount() { return mPeriodSize; }
    unsigned int getSampleRate() { return mSampleRate; }
    size_t getSampleSize(){ return mFrameSize; }
};
}
}
#endif

