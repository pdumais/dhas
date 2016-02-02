#pragma once

#include "IDWN.h"

class SprinklersNode: public IDWN
{
private:
    uint8_t mValveStatus;
public:
    SprinklersNode();
    ~SprinklersNode();

    bool processData(char* buf, size_t size, Dumais::JSON::JSON& reply);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);

    void valveOn_callback(RESTContext* context);
    void valveOff_callback(RESTContext* context);
    void status_callback(RESTContext* context);

};
