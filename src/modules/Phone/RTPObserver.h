#ifndef RTPOBSERVER_H
#define RTPOBSERVER_H

class Call;

class RTPObserver
{
public:
    virtual void onRTPSessionSoundQueueEmpty(Call *call)=0;
};


#endif

