#ifndef IPSERIALPORT_H
#define IPSERIALPORT_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "ISerialPort.h"

class IPSerialPort: public ISerialPort {
private:
    bool mFailedReconnect;
    time_t mLastReconnect;
    int mSocket;

protected:
    int mPort;
    std::string mAddress;

public:
	IPSerialPort();
	IPSerialPort(std::string ip, int port);
	virtual ~IPSerialPort();

    int Write(unsigned char *buf, int size);
    int Read(unsigned char* buf, int maxSize);

    bool Reconnect();
};

#endif

