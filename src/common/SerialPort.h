#ifndef SERIALPORT_H
#define SERIALPORT_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "ISerialPort.h"

class SerialPort: public ISerialPort {
private:
    int mSerialPortHandle;
public:
	SerialPort(char* serialPort);
	~SerialPort();

    int Write(unsigned char *buf, int size);
    int Read(unsigned char* buf, int maxSize);
    bool Reconnect();

};

#endif

