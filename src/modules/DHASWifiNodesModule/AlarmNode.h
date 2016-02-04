#pragma once

#include "IDWN.h"

class AlarmNode: public IDWN
{
private:
    uint8_t mPgms;

public:
    AlarmNode();
    ~AlarmNode();

    bool processData(char* buf, size_t size, Dumais::JSON::JSON& reply);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);

    void status_callback(RESTContext* context);
};
