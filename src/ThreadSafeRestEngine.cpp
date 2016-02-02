#include "ThreadSafeRestEngine.h"

void ThreadSafeRestEngine::addCallBack(std::string uri, std::string method, RESTCallBack* p)
{
    restEngine.addCallBack(uri,method,p);
}

void ThreadSafeRestEngine::removeCallBack(RESTCallBack* p)
{
    restEngine.removeCallBack(p);
}
RESTEngine::ResponseCode ThreadSafeRestEngine::invoke(Dumais::JSON::JSON& j,std::string url, const std::string& method, const std::string& data) const
{
    return restEngine.invoke(j,url, method, data);
}
void ThreadSafeRestEngine::documentInterface(Dumais::JSON::JSON& j)
{
    restEngine.documentInterface(j);
}
