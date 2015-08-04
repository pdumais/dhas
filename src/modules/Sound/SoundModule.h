#ifndef SOUNDSERVICE_H
#define SOUNDSERVICE_H
#include "Module.h"
#include <queue>
#include <string>
#include "SoundCard.h"

/*
 Sounnd names provided must be names only. This object will append ".wav" to it and will prefix it with "sounds/"
*/

class SoundModule: public Module{
private:
    Dumais::Sound::SoundCard *mpSoundDevice;
    std::string mSoundsFolder;
    
protected:
    virtual void configure(Dumais::JSON::JSON& config);
public:
	SoundModule();
	~SoundModule();

    void play_callback(RESTContext context);

    void abortPlayList();
    void registerCallBacks(RESTEngine* pEngine);    

    virtual void run();
    virtual std::string getName(){return "audio";}
    virtual void stop();
};

#endif

