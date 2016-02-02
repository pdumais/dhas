#pragma once
#include "utils/MPSCRingBuffer.h"

struct DataBuffer
{
    std::string destination;
    char data[256];
    unsigned char size;
};


class DWNSendQueue
{
private:
    Dumais::Utils::MPSCRingBuffer<DataBuffer>* mpSendQueue;
    int mSelfFD;

public:
    DWNSendQueue();
    ~DWNSendQueue();

    void sendToNode(const std::string& id, const char* data, unsigned char size);
    bool get(DataBuffer& data);
    int getSelfFD() {return this->mSelfFD; }
    
};
