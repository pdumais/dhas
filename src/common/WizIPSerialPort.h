#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "ISerialPort.h"

class WizIPSerialPort: public ISerialPort {
private:
    int mSocket;
    int mPort;
    std::string mAddress;

public:
	WizIPSerialPort(std::string mac);
	~WizIPSerialPort();

    int Write(unsigned char *buf, int size);
    int Read(unsigned char* buf, int maxSize);

    bool Reconnect();
};


