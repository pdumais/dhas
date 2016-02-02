#include "DHASLogging.h"
#include "DWNSendQueue.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>

using namespace Dumais::Utils;

DWNSendQueue::DWNSendQueue()
{
    mSelfFD = eventfd(0,EFD_NONBLOCK);
    mpSendQueue = new MPSCRingBuffer<DataBuffer>(500);
}

DWNSendQueue::~DWNSendQueue()
{
    close(mSelfFD);
    delete mpSendQueue;
}

void DWNSendQueue::sendToNode(const std::string& id, const char* data, unsigned char size)
{
    uint64_t tmp = 1;
    DataBuffer b;
    b.destination = id;
    memcpy(b.data,data,size);
    b.size = size;
    mpSendQueue->put(b);
    std::string dataStr(data,size);
    write(mSelfFD,&tmp,8);
    LOG("Pooled DWN message [" << dataStr << "] to " << id);
}

bool DWNSendQueue::get(DataBuffer& data)
{
    return this->mpSendQueue->get(data);
}

