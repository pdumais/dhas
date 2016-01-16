#pragma once
#include "Module.h"
#include <map>
#include "utils/MPSCRingBuffer.h"
#include <mutex>

struct DHASWifiNode
{
    std::string ip;
    std::string name;
    time_t lastHeartBeat;
    int socket;
};

struct DataBuffer
{
    std::string destination;
    char data[256];
    unsigned char size;
};

class DHASWifiNodesModule: public Module
{
private:
    int mSocket;
    int mEpollFD;
    int mSelfFD;
    std::map<std::string, DHASWifiNode> mNodes;
    Dumais::Utils::MPSCRingBuffer<DataBuffer>* mpSendQueue;
    std::mutex mNodesListLock;

    void discoverNode(std::string name, std::string ip);
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
    void registerCallBacks(RESTEngine* pEngine);
};


