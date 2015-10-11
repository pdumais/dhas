#include "utils/Logging.h"
#include "SilenceSound.h"
#include <sstream>

using namespace Dumais::Sound;

SilenceSound::SilenceSound(int duration, int samplingRate, int sampleSize)
{
    this->mDuration = duration;
    this->mSamplingRate = samplingRate;
    this->mSampleSize = sampleSize;
    this->mIndex = 0;
}

SilenceSound::~SilenceSound(){
}

int SilenceSound::getSample(int size, char* buf)
{
    int i=0;
    unsigned int maxSize = this->mSamplingRate*this->mDuration*this->mSampleSize;
    while (i<size && this->mIndex < maxSize)
    {
        buf[i]=0;
        this->mIndex++;
        i++;
    }

    return i;
}

std::string SilenceSound::toString()
{
    std::stringstream st;
    st<< this->mDuration << " seconds silence";
    return st.str();
}
