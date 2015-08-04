#include "Logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <sstream>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef SYSLOG
bool Logging::syslog = true;
#else
bool Logging::syslog = false;
#endif

bool Logging::disabled = false;

std::string Logging::syslogServer;

Logging::Logging()
{
}

Logging::~Logging(){
}

void Logging::log(const char *st,...)
{
    if (Logging::disabled) return;

    sockaddr_in s;
    int sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    memset(&s,0,sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(514);
    s.sin_addr.s_addr=inet_addr(Logging::syslogServer.c_str());

    char tmp[1024];
    va_list args;
    va_start(args,st);
    vsprintf((char*)&tmp,st,args);
    va_end(args);

    // strip leading \r\n
    for (int i=strlen((char*)tmp)-1;i>0;i--)
    {
        if (tmp[i]=='\r' || tmp[i]=='\n')
        {
            tmp[i]=0;
        } else {
            break;
        }
    }
    
    std::stringstream ss;
    ss<<"<190>"; // facility 23, level 6: 23*8 + 6 = 190
    ss <<" DHAS: ";
    ss<< tmp;
    const char* buf = ss.str().c_str();
    if (Logging::syslog)
    {
        sendto(sock,buf,ss.str().size(),0,(struct sockaddr *)&s,sizeof(s));
    }
    else
    {
        printf("%s\r\n",buf);
    }
    close(sock);

}

