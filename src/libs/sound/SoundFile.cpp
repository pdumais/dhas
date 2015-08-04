#include "SoundLogging.h"
#include "SoundFile.h"
#include <stdio.h>

#define ENCODING_MICROSOFT_ULAW 0x07 //http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/RIFF.html
#define ENCODING_MICROSOFT_PCM 0x01 //http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/RIFF.html

using namespace Dumais::Sound;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

struct riffHeader
{
    uint32_t chunckID;
    uint32_t chunkSize;
    uint32_t format;
    uint32_t subchunkID;
    uint32_t subChuckSize;
    uint16_t audioFormat;
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;    
    uint32_t subchunk2ID;
    uint32_t subChuck2size;

};

SoundFile::SoundFile(bool repeat)
{
    this->mRepeat = repeat;
    this->mSoundFile = 0;
}

SoundFile::~SoundFile()
{
    if (this->mSoundFile) fclose(this->mSoundFile);
}

bool SoundFile::open(std::string filename, SoundFormat fmt)
{
    this->mSoundFile = fopen(filename.c_str(),"r");
    if (!this->mSoundFile) return false;

    riffHeader header;
    fread((char*)&header,1,44,this->mSoundFile);
    uint16_t format = header.audioFormat;
    uint16_t channels = header.channels;
    int sampleRate = header.sampleRate;
    uint16_t bits = header.bitsPerSample;

    bool supported = false;
    if (format == ENCODING_MICROSOFT_ULAW && channels==1 && bits==8 && sampleRate==8000 && fmt==G711)
    {
        supported = true;
    } else if (format == ENCODING_MICROSOFT_PCM && channels==2 && bits==16 && sampleRate==44100 && fmt==CD)
    {
        supported = true;
    } else {
        LOG("Unsupported file format " << format);
    }

    if (!supported)
    {
        // Sound should be encoded with  sox -V %s -r 8000 -c 1 -t wav -e u-law %s-ulaw.wav
        fclose(this->mSoundFile);
        this->mSoundFile = 0;
    }

    this->mFileName = filename;
    return (this->mSoundFile!=0);
}

int SoundFile::getSample(int size, char* buf)
{
    if (!this->mSoundFile) return 0;

    int n = fread(buf,1,size,this->mSoundFile);

//Logging::log("read %i bytes from file. Requested %i",n,size);       
    if (n<size)
    {
        rewind(this->mSoundFile);
    }
    return n;

}


std::string SoundFile::toString()
{
    return this->mFileName;
}

