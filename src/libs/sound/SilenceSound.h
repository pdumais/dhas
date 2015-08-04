#ifndef SILENCESOUND_H
#define SILENCESOUND_H

#include "ISound.h"

namespace Dumais{
namespace Sound{
class SilenceSound: public ISound{
private:
    int mDuration;
    int mSamplingRate;
    int mSampleSize;
    unsigned int mIndex;
public:
	SilenceSound(int duration, int samplingRate, int sampleSize); 
	virtual ~SilenceSound();

    int getSample(int size,char* buf);
    std::string toString();
    
};
}
}
#endif

