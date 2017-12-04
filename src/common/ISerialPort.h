#pragma once

class ISerialPort{
public:
    virtual int Write(unsigned char *buf, int size) = 0;
    virtual int Read(unsigned char* buf, int maxSize) = 0;
    virtual bool Reconnect() = 0;

};


