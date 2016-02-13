#include "ThreadSafeRestEngine.h"

void ThreadSafeRestEngine::addCallBack(std::string uri, std::string method, RESTCallBack* p)
{
    engineLock.lock();
    restEngine.addCallBack(uri,method,p);
    engineLock.unlock();
}

void ThreadSafeRestEngine::removeCallBack(RESTCallBack* p)
{
    engineLock.lock();
    restEngine.removeCallBack(p);
    engineLock.unlock();
}
RESTEngine::ResponseCode ThreadSafeRestEngine::invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data) const
{
    return restEngine.invoke(j,url, method, data);
}
void ThreadSafeRestEngine::documentInterface(Dumais::JSON::JSON& j)
{
    restEngine.documentInterface(j);
}
