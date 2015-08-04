#ifndef ISOUND_H
#define ISOUND_H
#include <string>

namespace Dumais{
namespace Sound{
enum SoundFormat
{
    G711,
    CD
};

class ISound
{
public:
    virtual ~ISound(){}
    virtual int getSample(int size, char* buf)=0;
    virtual std::string toString()=0;
};
}
}
#endif
