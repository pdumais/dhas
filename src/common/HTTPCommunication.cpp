#include "HTTPCommunication.h"
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include "regex.h"
#include "Logging.h"
#include "stdlib.h"
#include <sys/time.h>
#include <errno.h>

HTTPCommunication::HTTPCommunication(){
}

HTTPCommunication::~HTTPCommunication(){
}

bool HTTPCommunication::postURL(std::string server, std::string url, std::string data, std::string contentType, int port)
{
    std::stringstream ss;
    ss << "POST "<< url << " HTTP/1.1\r\n";
    ss << "Content-Length: " << data.size() << "\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << data;

    sockaddr_in sockadd;
    int sock=socket(AF_INET,SOCK_STREAM,0);
    memset(&sockadd,0,sizeof(sockadd));
    struct hostent *he;
    he = gethostbyname(server.c_str());
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(port);
    memcpy(&sockadd.sin_addr, he->h_addr_list[0], he->h_length);
    if (connect(sock,(struct sockaddr *)&sockadd, sizeof(sockadd)) >= 0)
    {
        send(sock,ss.str().c_str(),ss.str().size(),0);
        close(sock);
        return true;
    }
    else
    {
        return false;
    }


}


//Warning: This does not respect Transfer-Encoding properly. Its a big hack. It will only work for the thermostat
std::string HTTPCommunication::getURL(std::string server, std::string url, int port)
{
    std::string ret="";
    bool chunked = false;
    std::stringstream ss;
    ss << "GET "<< url << " HTTP/1.1\r\n";
    ss << "Host: " << server << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    regex_t preg, preg2;
    regcomp(&preg,"content-length: ([0-9]*)",REG_EXTENDED|REG_ICASE);
    regcomp(&preg2,"Transfer-Encoding: chunked",REG_EXTENDED|REG_ICASE);
    regmatch_t matches[3];


    sockaddr_in sockadd;
    int sock=socket(AF_INET,SOCK_STREAM,0);
    memset(&sockadd,0,sizeof(sockadd));
    struct hostent *he;
    he = gethostbyname(server.c_str());
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(port);
    memcpy(&sockadd.sin_addr, he->h_addr_list[0], he->h_length);
    bool content = false;
    std::string st;
    if (connect(sock,(struct sockaddr *)&sockadd, sizeof(sockadd)) >= 0)
    {
        struct timeval tv;
        tv.tv_sec=30;
        tv.tv_usec = 0;
        setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv,sizeof(struct timeval));

        send(sock,ss.str().c_str(),ss.str().size(),0);
        char buf[1024];
        for (int i=0;i<1024;i++) buf[i] = 0;
        int size;
        int contentlength = 0;
        while ((size=recv(sock,(char*)&buf,1024-1,0))>0)
        {

            st.append((char*)&buf[0],size);

            // this is ugly. But thermostat wont close connection automatically and doesnot provide a content-lenght
            size_t p1 = st.find("\r\n\r\n");
            if (p1!=std::string::npos)
            {
                content=true;
                int err = regexec(&preg2,st.c_str(),1,matches,0);
                if (err != REG_NOMATCH) chunked = true;
                err = regexec(&preg,st.c_str(),2,matches,0);
                if (err != REG_NOMATCH)
                {
                    char num[10];
                    int s1 = matches[1].rm_so;
                    int e1 = matches[1].rm_eo;
                    int size1 = e1-s1;
                    memcpy(num,(char*)&st.c_str()[s1],size1);
                    num[size1] = 0;
                    contentlength = atoi((char*)&num[0]);
                    Logging::log("Content-length=%i",contentlength);
                }

                if (st.find("\r\n0\r\n\r\n")!=std::string::npos && chunked)
                {
                    ret = st.substr(p1+8,std::string::npos); 
                    break;
                }


                if (!chunked)
                {
                    if ((st.size()-p1-4)>=contentlength)
                    {
                        ret = st.substr(p1+4,std::string::npos);
                        break;
                    }

                }
            }

        }
        if (size <0)
        {
            Logging::log("Receiving from %s returned %i: %s",server.c_str(),errno,(char*)&st[0]);
        }
        close(sock);
        regfree(&preg);
        regfree(&preg2);
        if (ret=="")  // this is a hack because ControlByWeb devices don't send a response header
        {
            ret = st;   
        }
        return ret;
    }
    else
    {
        Logging::log("Could not connect to %s",server.c_str());
        regfree(&preg);
        regfree(&preg2);
        return "";
    }
}

