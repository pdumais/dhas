#pragma once
typedef unsigned int InsteonID;

class InsteonCommandInterface
{
public:
    virtual void writeCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2)=0;
    virtual void writeExtendedCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char* data)=0;
};
