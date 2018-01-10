#pragma once
#include <map>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>

struct Wiz110srConfig
{
    char code[4];
    unsigned char mac[6];
    unsigned char op;
    unsigned char ip[4];
    unsigned char subnet[4];
    unsigned char gateway[4];
    unsigned short port;
    char remotehost[4];
    unsigned short remoteport;

    //0xA0: 1200
    //0xD0: 2400 
    //0xE8: 4800
    //0xF4: 9600
    //0xFA: 19200
    //0xFD: 38400
    //0xFE: 57600
    //0xFF: 115200
    //0xBB: 230400
    unsigned char baud;
    char dbit;
    char parity;
    char sbit;
    char flow;
    char packing_char;
    unsigned short packing_length;
    unsigned short packing_time;
    unsigned short inactivity;
    char debug;
    unsigned short version;
    char dhcp;
    char udp;
    char conn;
    char domainflag;
    char domainname[32];
    char dns[4];
    char serial_config;
    char serial_trigger[3];
    char pppoeid[32];
    char pppoepassword[32];
    char usepassword;
    char connectionpassword[8];
} __attribute__((__packed__));

class Wiz110sr
{
private:
    struct sockaddr_in caddr;
    int client;
public:
    Wiz110sr();
    ~Wiz110sr();
    std::map<std::string,Wiz110srConfig> search(); 
    bool configure(std::string mac, Wiz110srConfig conf); 
};
