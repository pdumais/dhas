#pragma once
#include "rest/RESTEngine.h"
#include "rest/RESTCallBack.h"
#include <mutex>

class ThreadSafeRestEngine
{
private:
    RESTEngine restEngine;
    std::mutex engineLock;
public:
    void addCallBack(std::string uri, std::string method, RESTCallBack* p);
    void removeCallBack(RESTCallBack* p);
    RESTEngine::ResponseCode invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data) const;
    void documentInterface(Dumais::JSON::JSON& j);

};
