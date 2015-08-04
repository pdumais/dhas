#include "SoundLogging.h"
#include "AlsaPCM.h"


using namespace Dumais::Sound;
int recover(snd_pcm_t *handle, int error)
{
    if (error != -EPIPE && error != -ESTRPIPE) return error;

    if (error == -ESTRPIPE)
    {
        while ((error = snd_pcm_resume(handle)) == -EAGAIN) sleep(1);
        if (error >= 0) return 0;
    }
    snd_pcm_prepare(handle);
    return 0;
}

AlsaPCM::AlsaPCM(const char *device)
{
    snd_pcm_hw_params_t *params;
    int dir;
    int err;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    LOG("Using device " << device);
	err = snd_pcm_open(&mpDeviceHandle, device,SND_PCM_STREAM_PLAYBACK,0);
    if (err<0)
    {
        LOG("Playback open error: " << snd_strerror(err));
        mpDeviceHandle = 0;
        return;
    }
    
    snd_pcm_hw_params_alloca(&params);

    err=snd_pcm_hw_params_any(mpDeviceHandle, params);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_any " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }
    err=snd_pcm_hw_params_set_access(mpDeviceHandle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err<0) 
    {
        LOG("snd_pcm_hw_params_set_access " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }
    err=snd_pcm_hw_params_set_format(mpDeviceHandle, params,format);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_set_format " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }
    unsigned int rate=44100;
    err=snd_pcm_hw_params_set_rate_near(mpDeviceHandle, params,&rate,&dir);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_set_rate_near " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }

    err=snd_pcm_hw_params_set_channels(mpDeviceHandle,params,  2);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_set_channels " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }

    unsigned long bufferSize = 1024; // size in frames
    // buffer size is the ringbuffer. must be greater than periodSize since
    // it should always have one full period available 
    err=snd_pcm_hw_params_set_buffer_size_near(mpDeviceHandle, params,&bufferSize);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_set_buffer_size " << snd_strerror(err));
    }


    dir = 0;
    snd_pcm_uframes_t periodSize=512; 
    // this should be set to hald the buffersize. It is the number of frames between interrupts
    err=snd_pcm_hw_params_set_period_size_near(mpDeviceHandle, params,&periodSize,&dir);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_set_period_size "  << snd_strerror(err));
    }

    err=snd_pcm_hw_params(mpDeviceHandle,params);
    if (err<0)
    {
        LOG("snd_pcm_hw_params " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }

    snd_pcm_hw_params_get_buffer_size(params,&mBufferSize);
    snd_pcm_hw_params_get_period_size(params,&mPeriodSize, &dir);
    snd_pcm_hw_params_get_rate(params,&mSampleRate, &dir);

    unsigned int tmp;
    err = snd_pcm_hw_params_get_channels(params,&tmp);
    if (err<0)
    {
        LOG("snd_pcm_hw_params_get_channels " <<  err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }
    switch (format)
    {
        case SND_PCM_FORMAT_S16_LE:
            mFrameSize = tmp*2;
            break;
        default:
            mFrameSize = tmp*1;
    }
    LOG("Using " << tmp << " channels on sound card and buffersize of " << mBufferSize << " samples. Framesize=" << mFrameSize << ", periodSize=" << mPeriodSize << ", rate=" << mSampleRate);
    err=snd_pcm_prepare(mpDeviceHandle);
    if (err<0)
    {
        LOG("snd_pcm_prepare " << err);
        snd_pcm_drop(mpDeviceHandle);
        mpDeviceHandle = 0;
        return;
    }
}

AlsaPCM::~AlsaPCM()
{
    snd_pcm_drop(mpDeviceHandle);
}

void AlsaPCM::playChunk(char *buf, size_t numFrames)
{
    if (!mpDeviceHandle) return;


    int l;
    while (numFrames > 0) {
        l = snd_pcm_writei(mpDeviceHandle, buf, numFrames);
        if ((l == -EPIPE)||(l == -ESTRPIPE)) {
            if (recover(mpDeviceHandle, l) < 0) LOG("recovering failed");
            numFrames=0;
        } else if (l != -EAGAIN){
            buf += l * mFrameSize;
            numFrames -= l;
        }
    }
}
