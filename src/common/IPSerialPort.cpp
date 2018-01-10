#include "../DHASLogging.h"
#include "IPSerialPort.h"
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

IPSerialPort::IPSerialPort()
{
    this->mSocket = 0;
    this->mAddress = "";
    this->mPort = 0;
    this->mLastReconnect = 0;

    // We set this to true so that we can get the initial "connecting to" log
    this->mFailedReconnect = true;
}
IPSerialPort::IPSerialPort(std::string target, int port)
{
    this->mSocket = 0;
    this->mAddress = target;
    this->mPort = port;
    this->mLastReconnect = 0;

    // We set this to true so that we can get the initial "connecting to" log
    this->mFailedReconnect = true;
}

IPSerialPort::~IPSerialPort()
{
    if (this->mSocket >= 0)
    {
        close(this->mSocket);
        this->mSocket = 0;
    }
}

int IPSerialPort::Write(unsigned char *buf, int size)
{
    if (this->mSocket == 0) return -1;

    int r = write(this->mSocket,buf,size);
    if (r < 0)
    {
        int e = errno;
        if (e == EAGAIN || e == EWOULDBLOCK)
        {
            return -1;
        }
        else    
        {
            LOG("Write to IPSerialPort ("<<this->mAddress<<") returned " << e);
            close(this->mSocket);
            this->mSocket = 0;
            return -1;
        }
    }
}

int IPSerialPort::Read(unsigned char* buf, int maxSize)
{
    if (this->mSocket == 0) return -1;


    int r = read(this->mSocket,buf,maxSize);
    if (r < 0)
    {
        int e = errno;
        if (e == EAGAIN || e == EWOULDBLOCK)
        {
            return -1;
        }
        else    
        {
            LOG("Read from IPSerialPort ("<<this->mAddress<<") returned " << e);
            close(this->mSocket);
            this->mSocket = 0;
            return -1;
        }
    }
    else if (r == 0)
    {
        close(this->mSocket);
        this->mSocket = 0;
        return -1;
    }
    return r;
}

bool IPSerialPort::Reconnect()
{
    if (this->mSocket != 0 ) return true;
    time_t t;
    time(&t);

    // Throttle reconnections. No more than once per 2sec.
    if ((this->mLastReconnect+2) > t) return false;

    this->mLastReconnect = t;
    sockaddr_in sockadd;
    memset(&sockadd,0,sizeof(sockadd));

    mSocket = socket(AF_INET,SOCK_STREAM,0);
    struct hostent *he;
    he = gethostbyname(this->mAddress.c_str());
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(this->mPort);
    memcpy(&sockadd.sin_addr, he->h_addr_list[0], he->h_length);

    // We only log this if there was a failure to connect before.
    // This is because some IP serial ports such as ESP-Link closes
    // the connection every 5 minutes and this would generate too many logs.
    // So we log only if we consider it a failure.
    if (this->mFailedReconnect)
    {
        LOG("Connecting to IP serial port at " << inet_ntoa(sockadd.sin_addr));
    }

    if (connect(this->mSocket,(struct sockaddr *)&sockadd, sizeof(sockadd)) == 0)
    {
        int flags = fcntl(this->mSocket,F_GETFL,0);
        fcntl(this->mSocket, F_SETFL, flags | O_NONBLOCK);
        this->mFailedReconnect = false;
        return true;
    }

    LOG("Error connecting to IP serial port at " << this->mAddress);
    close(this->mSocket);
    this->mSocket = 0;
    this->mFailedReconnect = true;
    return false;
}


