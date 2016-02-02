#pragma once

#include "IDWN.h"

class AlarmNode: public IDWN
{
public:
    AlarmNode();
    ~AlarmNode();

    bool processData(char* buf, size_t size, Dumais::JSON::JSON& reply);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
};
