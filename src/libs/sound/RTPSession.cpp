#include "utils/Logging.h"
#include "RTPSession.h"
#include "SoundFile.h"
#include "SilenceSound.h"
#include <sstream>


using namespace Dumais::Sound;

RTPSession::RTPSession()
{
    this->mSession=rtp_session_new(RTP_SESSION_SENDRECV);
    this->mLocalIP = "";
    this->mLocalPort=0;
    this->mMusic = 0;
    this->mCurrentSound = 0;
}

RTPSession::~RTPSession(){
    this->stop();

    // Clear the queue
    ISound *s;
    while(this->mSoundQueue.get(s)) delete s;
}


void RTPSession::start()
{
    rtp_session_set_local_addr(mSession,this->mLocalIP.c_str(),this->mLocalPort,0);
    rtp_session_set_scheduling_mode(mSession,1);
    rtp_session_set_blocking_mode(mSession,1);
    rtp_session_set_connected_mode(mSession,TRUE);
    rtp_session_set_remote_addr(mSession,this->mPeerIP.c_str(),this->mPeerPort);
    rtp_session_set_payload_type(mSession,0);

    if (this->mCurrentSound) delete this->mCurrentSound;
    this->mUser_ts = 0;
    this->mCurrentSound = 0;
    this->mQueueWasEmpty = true;
    this->mInterruptCurrentSound=false;
    this->mResetTime = false;

    this->mStopping = false;
    this->mThread = std::thread([this](){
        while (!mStopping)
        {
            this->work();
        }
    });
}

void RTPSession::stop()
{
    if (this->mThread.joinable())
    {
        this->mStopping = true;
        this->setWorking();
        this->mThread.join();
    }

    if (this->mSession!=0)
    {
        rtp_session_destroy(this->mSession);
        this->mSession = 0;
    }
}

void RTPSession::setPeerAddress(std::string ip, unsigned int peerPort)
{
    this->mPeerIP = ip;
    this->mPeerPort= peerPort;
}

void RTPSession::setLocalAddress(std::string ip, unsigned int port)
{
    this->mLocalPort = port;
    this->mLocalIP = ip;
}

unsigned int RTPSession::getLocalPort()
{
    return this->mLocalPort;
}

std::string RTPSession::getLocalIP()
{
    return this->mLocalIP;
}

void RTPSession::work()
{
    if (!mCurrentSound)
    {
        if (this->mSoundQueue.get(mCurrentSound))
        {
            mQueueWasEmpty=false;
            this->mInterruptCurrentSound=false;
        } else {
            if (!mQueueWasEmpty) // in order to notify once only.
            {
                this->notifyQueueEmpty();
                mQueueWasEmpty=true;
            }
        }
    }

    if (!this->mMusic && !this->mCurrentSound)
    {
        //TODO: this will suppress silence. Should not set idle if we do not want silence suppression
        this->setIdle();    
        this->mResetTime = true;
        return;
    }

    int soundindex = 0;
    bool silenceSuppresion = true;
    if (this->mMusic)
    {
        this->mMusic->getSample(CONF_RTP_PAYLOAD_SIZE,(char*)&mBuffer);
        silenceSuppresion=false;
    }

    int numberOfBytesToSend = CONF_RTP_PAYLOAD_SIZE;
    if (mCurrentSound)   // if there is a sound to be played from queue, hide the music
    {
        silenceSuppresion= false;
        // overwrite music with sound
        soundindex=mCurrentSound->getSample(CONF_RTP_PAYLOAD_SIZE,(char*)&mBuffer);

        if (soundindex<CONF_RTP_PAYLOAD_SIZE || this->mInterruptCurrentSound)
        {
            delete mCurrentSound;
            mCurrentSound = 0;
        
            if (this->mMusic)
            {
               //Note: if buffer wasn't filled completely, it contains music anyways.
            }
            else
            {
                numberOfBytesToSend = soundindex;
            }
        }
    }

    if (!silenceSuppresion)
    {
        if (mResetTime)
        {
            mUser_ts = rtp_session_get_current_send_ts(this->mSession);
            mResetTime = false;
        }
        rtp_session_send_with_ts(this->mSession,(uint8_t*)&mBuffer,numberOfBytesToSend,mUser_ts);
        mUser_ts+=numberOfBytesToSend;
    } else {
        // we started skipping frames, so we will need to resync next time
        mResetTime = true;
    }
}

