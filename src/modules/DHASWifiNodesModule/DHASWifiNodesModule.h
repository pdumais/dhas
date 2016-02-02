#pragma once
#include "Module.h"
#include <map>
#include <mutex>
#include "IDWN.h"
#include "DWNSendQueue.h"

struct DHASWifiNode
{
    int socket;
    time_t lastHeartBeat;
    IDWN* driver;
};

class DHASWifiNodesModule: public Module
{
private:
    int mSocket;
    int mEpollFD;
    std::map<std::string, DHASWifiNode> mNodes;
    std::mutex mNodesListLock;
    DWNSendQueue mSendQueue;
    ThreadSafeRestEngine* mpRestEngine;

    void discoverNode(std::string name, std::string ip, std::string id);
    bool connectNode(DHASWifiNode* node);
    void checkHeartBeats();
    void receiveFromNode(DHASWifiNode* node);
    void sendToNode(const std::string& id, char* data, unsigned char size);  /* ID is the node's IP*/  
    void addFdToEpoll(int fd);
    bool processDataFromMulticastSocket();
    void closeAllNodes();

protected:
    virtual void configure(Dumais::JSON::JSON& config);

public:
	DHASWifiNodesModule();
	~DHASWifiNodesModule();

    virtual void stop();
    virtual void run();
    virtual std::string getName(){return "DHASWifiNodes";}

    void sendRaw_callback(RESTContext* context);
    void getDevices_callback(RESTContext* context);
    void registerCallBacks(ThreadSafeRestEngine* pEngine);
};


