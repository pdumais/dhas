#include "DHASLogging.h"
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <sstream>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

SyslogLogging::SyslogLogging(const std::string& server)
{
    this->syslogServer = server;
}

SyslogLogging::~SyslogLogging()
{
}

void SyslogLogging::log(const std::string& str)
{
    sockaddr_in s;
    int sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    memset(&s,0,sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(514);
    s.sin_addr.s_addr=inet_addr(this->syslogServer.c_str());

    std::stringstream ss;
    ss<<"<190>"; // facility 23, level 6: 23*8 + 6 = 190
    ss <<" DHAS: ";
    ss<< str;

    std::string logstr = ss.str();
    const char* buf = logstr.c_str();

    sendto(sock,buf,logstr.size(),0,(struct sockaddr *)&s,sizeof(s));
    close(sock);

}

