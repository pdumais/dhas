#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "IPSerialPort.h"

class WizIPSerialPort: public IPSerialPort {
public:
	WizIPSerialPort(std::string mac);
	virtual ~WizIPSerialPort();
};


