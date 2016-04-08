#pragma once

#include "IDWN.h"
#include "dumaislib/utils/MPSCRingBuffer.h"

typedef std::pair<unsigned int,unsigned char> SequenceItem;

class SprinklersNode: public IDWN
{
private:
    uint8_t mValveStatus;
    Dumais::Utils::MPSCRingBuffer<SequenceItem>* mpSequence;
    time_t mNextSequenceTime;
    std::string mCurrentSequence;

    void clearSequence();
    void addInSequence(unsigned char valve, unsigned int duration);

public:
    SprinklersNode();
    ~SprinklersNode();

    bool processData(char* buf, size_t size, Dumais::JSON::JSON& reply);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
    virtual void run(time_t t);
    virtual void addInfo(Dumais::JSON::JSON& obj);

    void sequence_callback(RESTContext* context);
    void valveOn_callback(RESTContext* context);
    void valveOff_callback(RESTContext* context);
    void status_callback(RESTContext* context);

};
