#include "Wiz110sr.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <unistd.h>

Wiz110sr::Wiz110sr()
{
    if ((client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0) 
    {
        return;
    }

    int broadcast = 1;
    setsockopt(client, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);

    struct sockaddr_in addr;
    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(5001);

    caddr.sin_family = AF_INET;
    caddr.sin_port = htons(1460);
    caddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    if (bind(client, (struct sockaddr *)&addr, sizeof(addr))<0) 
    {
        close(client);
        client = 0;
        return;
    }

}

Wiz110sr::~Wiz110sr()
{
    if (client) close(client);
    client = 0;
}

std::map<std::string,Wiz110srConfig> Wiz110sr::search()
{
    std::map<std::string,Wiz110srConfig> confs;

    if (client == 0) return confs;

    unsigned char dataSend[4] = {0x46,0x49,0x4E,0x44};
    if (sendto(client, dataSend, 4, 0, (sockaddr *)&caddr, sizeof(caddr)) < 0)
    {
        close(client);
        client = 0;
        return confs;
    }
    
    while (1)
    {
        Wiz110srConfig conf;
        int r = recv(client,&conf,sizeof(conf),0);
        if (r <= 0) break;

        std::stringstream mac;
        mac << std::hex 
            << std::setfill('0') << std::setw(2) << (int)conf.mac[0] << ":" 
            << std::setfill('0') << std::setw(2) << (int)conf.mac[1] << ":" 
            << std::setfill('0') << std::setw(2) << (int)conf.mac[2] << ":" 
            << std::setfill('0') << std::setw(2) << (int)conf.mac[3] << ":" 
            << std::setfill('0') << std::setw(2) << (int)conf.mac[4] << ":" 
            << std::setfill('0') << std::setw(2) << (int)conf.mac[5];
        confs[mac.str()] = conf;
    }
    
    return confs;
}

bool Wiz110sr::configure(std::string mac, Wiz110srConfig conf)
{
    if (client == 0) return false;

    conf.code[0] = 'S';
    conf.code[1] = 'E';
    conf.code[2] = 'T';
    conf.code[3] = 'T';

    if (sendto(client, &conf, sizeof(conf), 0, (sockaddr *)&caddr, sizeof(caddr)) < 0)
    {
        close(client);
        client = 0;
        return false;
    }

    while (1)
    {
        Wiz110srConfig response;
        int r = recv(client,&response,sizeof(response),0);
        if (r <= 0) return false;
    
        bool isThisDevice = true;
        for (int i=0;i<6;i++) if (response.mac[i] != conf.mac[i]) isThisDevice = false;
        if (!isThisDevice) continue;
        if (response.code[3] != 'C') return false;
        return true;
    }
}
