#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include "ISound.h"
#include <string>

namespace Dumais{
namespace Sound{

class SoundFile: public ISound{
private:
    FILE *mSoundFile;   
    std::string mFileName;
    bool mRepeat;
public:
	SoundFile(bool repeat=false);
	~SoundFile();

    bool open(std::string filename,SoundFormat fmt);
    int getSample(int size, char* buf);
    std::string toString();
};
}
}
#endif

