#pragma once

namespace Dumais{
namespace Sound{

class ISoundPlaylistObserver
{
public:
    ~ISoundPlaylistObserver(){}
    virtual void onSoundQueueEmpty()=0;
};

}
}
