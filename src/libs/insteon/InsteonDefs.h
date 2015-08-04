#pragma once
typedef unsigned int InsteonID;

struct InsteonDirectMessage
{
    bool extended;
    unsigned char cmd1;
    unsigned char cmd2;
    unsigned char data[14];
};

struct InsteonMessage
{
    unsigned char size;
    unsigned char data[25];
    unsigned char expectedEchoSize;
};

struct InsteonCommand
{
    unsigned char stx;
    unsigned char cmd;
    InsteonID     destination;
    unsigned char flags;
    unsigned char cmd1;
    unsigned char cmd2;
    unsigned char data[14];
};

