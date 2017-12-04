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
    int mSocket;
    int mPort;
    std::string mAddress;

public:
	IPSerialPort(std::string mac);
	~IPSerialPort();

    int Write(unsigned char *buf, int size);
    int Read(unsigned char* buf, int maxSize);

    bool Reconnect();
};

#endif

