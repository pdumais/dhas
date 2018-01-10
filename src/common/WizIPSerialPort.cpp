#include "../DHASLogging.h"
#include "WizIPSerialPort.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include "Wiz110sr.h"
#include <iostream>

// This is to connect to Insteon Modem through a Wiznet110sr IP module.
// the device needs to be configured to 19200 baud through the windows 
// wiznet configuration application. We could do it in here but
// the protocol is not documented. Although it would be simple to reverse
// engineer.

WizIPSerialPort::WizIPSerialPort(std::string target): IPSerialPort()
{
    Wiz110sr w;
    std::map<std::string,Wiz110srConfig> confs = w.search();
    if (confs.count(target))
    {
        Wiz110srConfig wc = confs[target];
       
        if (wc.baud != 0xFA) 
        {
            //TODO: That's not good because the device will reset so should wait
            // for it to come back up before using it
            LOG("Found Wiznet device but it needs configuration");
            wc.baud = 0xFA; //19200
            if (!w.configure(target,wc))
            {
                LOG("Could not configure wiznet device");
                return;
            }
        } 

        std::stringstream ss;
        ss << (int)wc.ip[0] << "." << (int)wc.ip[1] << "." << (int)wc.ip[2] << "." << (int)wc.ip[3];
        this->mAddress = ss.str();
        this->mPort = __builtin_bswap16(wc.port);
        LOG("Found wiznet device at " << this->mAddress << ":" <<this->mPort);
    }
    else
    {
        LOG("Could not find wiznet device");   
    }
}

WizIPSerialPort::~WizIPSerialPort(){
}

