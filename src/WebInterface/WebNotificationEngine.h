#ifndef WEBNOTIFICATIONENGINE_H
#define WEBNOTIFICATIONENGINE_H
#include <map>
#include <pthread.h>
#include <string>
#include <JSON.h>
#include "IEventNotificationListener.h"

class WebNotificationEngine: public IEventNotificationListener
{
private:
    void sendData(int socket, const std::string& data);
    std::map<int,bool> mSocketList;
    pthread_mutex_t mSocketListLock;
public:
	WebNotificationEngine();
	~WebNotificationEngine();

    virtual void notifyEvent(const Dumais::JSON::JSON& jsonEvent);
    void unsubscribe(int socket);
    void subscribe(int socket);
    void activateSubscription(int socket);
};

#endif

