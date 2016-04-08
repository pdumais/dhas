#include "DHASWifiNodesModule.h"
#include "DHASLogging.h"
#include <list>
#include <sstream>
#include "json/JSON.h"
#include "ModuleRegistrar.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>

#define NODE_HEARTBEAT 5    /* 5 seconds */
#define MAX_NODES_COUNT 20

using namespace Dumais::Utils;

REGISTER_MODULE(DHASWifiNodesModule)

DHASWifiNodesModule::DHASWifiNodesModule()
{
    mpRestEngine = 0;
}

DHASWifiNodesModule::~DHASWifiNodesModule()
{
}

void DHASWifiNodesModule::configure(Dumais::JSON::JSON& config)
{
}

void DHASWifiNodesModule::stop()
{
    LOG("Attempting to shutdown DHASWifiNodesModule server");

    // Closing the multicast socket
    if (mSocket) shutdown(mSocket,SHUT_RDWR);
    mSocket = 0;
}

void DHASWifiNodesModule::closeAllNodes()
{
    for (auto& it : mNodes)
    {
        close(it.second.socket);
    }
}

void DHASWifiNodesModule::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack *p;
    p = new RESTCallBack(this,&DHASWifiNodesModule::sendRaw_callback,"send raw data to wifi node");
    p->addParam("destination","IP address of the node",false);
    p->addParam("data","raw data to be sent",false);
    pEngine->addCallBack("/dwn/sendraw","GET",p);

    p = new RESTCallBack(this,&DHASWifiNodesModule::getDevices_callback,"list connected wifi nodes");
    pEngine->addCallBack("/dwn/list","GET",p);

    mpRestEngine = pEngine;

}

void DHASWifiNodesModule::receiveFromNode(DHASWifiNode* node)
{

    //TODO: we could be receiving two messages in the same packet. we should accumulate in buffer until we find a terminating 0.
    //      and all DWN devices should terminate a message with 0
    char buf[2000];
    int n = recv(node->socket,buf,2000,0);
    //LOG("receiving from node: " << n << "  " << errno);
    if (n<=0)
    {
        if (n==-1 && errno == EAGAIN) return;

        LOG("Node failed [" << node->driver->getName() << "] @ " << node->driver->getInfo().ip << " (" << errno << "). Removing");
        close(node->socket);

        Dumais::JSON::JSON json;
        json.addValue("dwn","event");
        json.addValue("closed","type");
        json.addValue(node->driver->getInfo().ip,"ip");
        json.addValue(node->driver->getName(),"name");
        json.addValue(node->driver->getInfo().id,"id");
        mpEventProcessor->processEvent(json);

        std::string ip = node->driver->getInfo().ip;
        node->driver->unRegisterCallBacks(mpRestEngine);
        delete node->driver;
        node->driver = 0;

        this->mNodesListLock.lock();
        mNodes.erase(ip);
        this->mNodesListLock.unlock();
        return;
    }

    std::string payload (buf,n);
    //LOG("payload (" << n << "): " << payload);
    if (payload == "yo!\r\n")
    {
        time(&node->lastHeartBeat);
    }
    else
    {  
        if (node->driver == 0)
        {
            LOG("ERROR: Can't process data from " << node->driver->getName() <<" ("<<node->driver->getInfo().id<<". Driver not associated");
        }
        else
        { 
            Dumais::JSON::JSON j;
            if (node->driver->processData(buf,n,j))
            {
                j.addValue("dwn","event");
                j.addValue(node->driver->getInfo().ip,"ip");
                j.addValue(node->driver->getName(),"name");
                j.addValue(node->driver->getInfo().id,"id");
                mpEventProcessor->processEvent(j);
            }
        }
    }
}

// Give some cycles to all nodes periodically
void DHASWifiNodesModule::runNodes()
{
    time_t t;
    time(&t);
    for (auto& it : mNodes)
    {
        DHASWifiNode* node = &it.second;
        node->driver->run(t);
    }
}

void DHASWifiNodesModule::checkHeartBeats()
{
    time_t t;
    time(&t);
    t -= (3*NODE_HEARTBEAT);
    for (auto& it : mNodes)
    {
        DHASWifiNode* node = &it.second;
        if (node->lastHeartBeat <= t)
        {
            LOG("Node [" << node->driver->getName() <<"] @ " << node->driver->getInfo().ip << " heartbeat failure. Disconnecting");
            close(node->socket);
    
            Dumais::JSON::JSON json;
            json.addValue("dwn","event");
            json.addValue("closed","type");
            json.addValue(node->driver->getInfo().ip,"ip");
            json.addValue(node->driver->getName(),"name");
            json.addValue(node->driver->getInfo().id,"id");
            mpEventProcessor->processEvent(json);

            std::string ip = node->driver->getInfo().ip;
            node->driver->unRegisterCallBacks(mpRestEngine);
            delete node->driver;
            node->driver = 0;

            this->mNodesListLock.lock();
            mNodes.erase(ip);
            this->mNodesListLock.unlock();
        }
    }
}


bool DHASWifiNodesModule::connectNode(DHASWifiNode* node)
{
    node->socket = socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in sockadd;
    memset(&sockadd,0,sizeof(sockadd));

    struct hostent *he;
    he = gethostbyname(node->driver->getInfo().ip.c_str());
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(242);
    memcpy(&sockadd.sin_addr, he->h_addr_list[0], he->h_length);

    LOG("Connecting to wifi node at " << inet_ntoa(sockadd.sin_addr));
    if (connect(node->socket,(struct sockaddr *)&sockadd, sizeof(sockadd)) == 0)
    {
        int flags = fcntl(node->socket,F_GETFL,0);
        fcntl(node->socket, F_SETFL, flags | O_NONBLOCK);
        addFdToEpoll(node->socket);
        return true;
    }
    
    
    close (node->socket);
    node->socket = 0;
    return false;
}

void DHASWifiNodesModule::addFdToEpoll(int fd)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(mEpollFD, EPOLL_CTL_ADD, fd, &ev); 
}

void DHASWifiNodesModule::discoverNode(std::string name, std::string ip, std::string id)
{
    if (ip == "0.0.0.0") return;

    if (mNodes.find(ip) == mNodes.end())
    {
        LOG("Found an advertisement from [" << name << "] at " << ip);
        if (mNodes.size() >= MAX_NODES_COUNT)
        {
            LOG("ERROR: The number of nodes registered in the system has reached maximum capacity");
            return;
        }
    
        IDWN* driver = IDWN::createDriverInstance(name);
        if (driver == 0)
        {
            // TODO: should probably ignore all further broadcasts.
            LOG("ERROR: No suitable driver found for wifi node [" << name << "]");
            return;
        }

        DHASWifiNode node;
        node.driver = driver;
        node.driver->getInfo().ip = ip;
        node.driver->getInfo().id = id;
        node.driver->getInfo().sendQueue = &mSendQueue;
        time(&node.lastHeartBeat);

        if (connectNode(&node))
        {
            LOG("Node [" << name << "] @ " << ip << " connected.");
            this->mNodesListLock.lock();
            mNodes[ip] = node;
            this->mNodesListLock.unlock();
            
            node.driver->registerCallBacks(mpRestEngine);

            Dumais::JSON::JSON json;
            json.addValue("dwn","event");
            json.addValue("new","type");
            json.addValue(ip,"ip");
            json.addValue(name,"name");
            json.addValue(id,"id");
            mpEventProcessor->processEvent(json);
        }
        else
        {
            LOG("Could not connect to node [" << name << "] @ " << ip);
        }
    }
}

bool DHASWifiNodesModule:: processDataFromMulticastSocket()
{
    char buf[2000];
    struct sockaddr_in src;
    socklen_t addrlen;

    addrlen = sizeof(struct sockaddr_in);
    int n = recvfrom(mSocket, buf, 2000, 0, (struct sockaddr*)&src, &addrlen);
    if(n<0)
    {
        if (errno == EAGAIN) return true; // Why would this happen?
        close(mSocket);
        LOG("Multicast listener read returned " << n << "(" << errno << "). Socket closed.");
        return false;
    }
    else if (n>0)
    {
        std::string str(buf,n);
        Dumais::JSON::JSON json;
        json.parse(str);
        char* ip = inet_ntoa(src.sin_addr);
        this->discoverNode(json["name"].str(),ip,json["id"].str());
    }

    return true;
}

void DHASWifiNodesModule::run()
{
    DataBuffer data;
    struct epoll_event *events;
    events = new epoll_event[MAX_NODES_COUNT + 2];
    mEpollFD = epoll_create1(0);

    std::vector<std::string> submodules = IDWN::getRegisteredSubModulesNames();
    for (auto& it : submodules)
    {
        LOG("Registered DWN driver for ["<<it<<"]");
    }

    mSocket = socket(AF_INET, SOCK_DGRAM, 0);

    int tmp = 1;
    if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&tmp, sizeof(tmp)))
    {
        LOG("Could not setsockopt on multicast listener socket");
        close(mSocket);
        return;
    }

    struct sockaddr_in sockaddr = {0};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(242);
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(mSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)))
    {
        LOG("Could not bind multicast listener socket");
        close(mSocket);
        return;
    }

    struct ip_mreq group;
    group.imr_multiaddr.s_addr = inet_addr("224.242.242.242");
    group.imr_interface.s_addr = inet_addr("0.0.0.0");
    if (setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(group)) < 0)
    {
        close(mSocket);
        LOG("Could not join multicast group");
        return;
    }

    int flags = fcntl(mSocket,F_GETFL,0);
    fcntl(mSocket, F_SETFL, flags | O_NONBLOCK);

    addFdToEpoll(mSendQueue.getSelfFD());
    addFdToEpoll(mSocket);
    setStarted();
    while (!stopping())
    {
        int n = epoll_wait(mEpollFD, events, MAX_NODES_COUNT + 2, 1000);
        for (int i = 0; i < n; i++)
        {   
            if (events[i].data.fd == mSendQueue.getSelfFD())
            {
                // This was just to interrupt the epoll_wait. Ignore data
                char tmp[8];
                read(events[i].data.fd,tmp,8);
            }
            else if (events[i].data.fd == mSocket)
            {
                if (!processDataFromMulticastSocket()) return;
            }     
            else
            {
                for (auto& it : mNodes)
                {
                    if (it.second.socket == events[i].data.fd)
                    {
                        receiveFromNode(&it.second);
                        break;
                    }
                }
            }
        } 

        while (this->mSendQueue.get(data))
        {
            if (mNodes.find(data.destination) != mNodes.end())
            {
                LOG("Sending data to " << data.destination);
                send(mNodes[data.destination].socket,data.data,data.size,0);
            }
        }

        this->checkHeartBeats();
        this->runNodes();
    }

    if (mSocket) close(mSocket);
    closeAllNodes();
}

void DHASWifiNodesModule::getDevices_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    json.addList("modules");

    this->mNodesListLock.lock();
    for (auto& it : mNodes)
    {
        Dumais::JSON::JSON& obj = json["modules"].addObject("module");
        obj.addValue(it.second.driver->getName(),"name");
        obj.addValue(it.second.driver->getInfo().ip,"ip");
        obj.addValue(it.second.driver->getInfo().id,"id");
        it.second.driver->addInfo(obj);
    }
    this->mNodesListLock.unlock();
}

void DHASWifiNodesModule::sendRaw_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    std::string ip = params->getParam("destination");
    std::string data = params->getParam("data");
    mSendQueue.sendToNode(ip,data.c_str(),data.size());
    json.addValue("ok","status");
    return;
}

