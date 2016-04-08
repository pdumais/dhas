#pragma once

#include "IDWN.h"

class IOBoard: public IDWN
{
private:
    uint32_t mPgms;
    uint8_t  mPgmCount;

public:
    IOBoard();
    ~IOBoard();

    bool processData(char* buf, size_t size, Dumais::JSON::JSON& reply);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
    virtual void run(time_t t);

    void status_callback(RESTContext* context);
    void setoutput_callback(RESTContext* context);
};
