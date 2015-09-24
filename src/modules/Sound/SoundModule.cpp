#include "Logging.h"
#include "SoundModule.h"
#include "SoundFile.h"
#include "SoundLogging.h"
#include <stdio.h>
#include "SoundListParser.h"
#include "ModuleRegistrar.h"
#include "SoundDeviceFactory.h"

#define PLAYSTRING_DESCRIPTION "Coma separated list of files to play. digits can be used. They will be decoded and the proper sound files will be constructed. I.e: sound1,29,4,sound2 would play sound files: sound1.wav,20.wav,9.wav,4.wav,sound2.wav Note that number reconstruction only work for french grammar and only supports numbers -59..59 inclusively. For playing silence, you can use the number of seconds prefixed by a dollar sign. I.e: sound1$4,sound2. This would make a 4 second pause between sound1 and sound2"

REGISTER_MODULE(SoundModule)

class SoundLogger: public Dumais::Sound::ILogger
{
    virtual void log(const std::string& ss)
    {
        Logging::log("%s",ss.c_str());
    }
};

SoundModule::SoundModule()
{
    Dumais::Sound::Logging::logger = new SoundLogger();
}

SoundModule::~SoundModule()
{
    if (mpSoundDevice) delete mpSoundDevice;
}

void SoundModule::configure(Dumais::JSON::JSON& config)
{

    mSoundsFolder = config["soundsfolder"].str();
    Dumais::Sound::SoundDeviceFactory f;    
    mpSoundDevice = f.createSoundCard(config["sounddevice"].str());
}

void SoundModule::play_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    Logging::log("Play sound [%s]",params->getParam("sound").c_str());
    SoundListParser slp(params->getParam("sound"));
    std::vector<std::string> list = slp.getSoundList();
    for (std::vector<std::string>::iterator it = list.begin();it!=list.end();it++)
    {
        if ((*it)[0]=='$')
        {
            int s = std::stoi((*it).substr(1));
            mpSoundDevice->silence(s);
        }
        else
        {
            std::string st = mSoundsFolder+"/"+(*it)+"-s16.wav";
            mpSoundDevice->play(st.c_str(), Dumais::Sound::CD);
        }
    }
}

void SoundModule::registerCallBacks(RESTEngine* pEngine)
{
    RESTCallBack *p;

    p = new RESTCallBack(this,&SoundModule::play_callback,"plays files defined by PLAY_STRING on onboard sound device");
    p->addParam("sound",PLAYSTRING_DESCRIPTION);
    pEngine->addCallBack("/audio/play","GET",p);
}


void SoundModule::run()
{
    mpSoundDevice->start();
    setStarted();
    while (!stopping())
    {
        //TODO: this thread is useless
        sleep(1);
    }
    mpSoundDevice->stop();
}

void SoundModule::stop()
{
}

void SoundModule::abortPlayList()
{
    if (!mpSoundDevice) return;

    mpSoundDevice->clearQueue();
}

