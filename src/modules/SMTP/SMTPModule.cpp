#include "SMTPModule.h"
#include "Logging.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <list>
#include <sstream>
#include "JSON.h"
#include <stdlib.h>
#include "AlarmState.h"
#include "ModuleRegistrar.h"

#define BUF_SIZE 1024
REGISTER_MODULE(SMTPModule)

SMTPModule::SMTPModule(){
}

SMTPModule::~SMTPModule(){
}

void SMTPModule::configure(Dumais::JSON::JSON& config)
{
    this->mBindPort = config["bind"].toInt();
}

void SMTPModule::stop()
{
    Logging::log("Attempting to shutdown SMTP server");
    if (mServerSocket) shutdown(mServerSocket,SHUT_RDWR);
}


void SMTPModule::registerCallBacks(RESTEngine* pEngine)
{
}


void SMTPModule::run()
{
    mServerSocket = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in sockadd;
    sockadd.sin_family=AF_INET;
    sockadd.sin_addr.s_addr=INADDR_ANY;
    sockadd.sin_port=htons(this->mBindPort);
    char r = 1;
    setsockopt(mServerSocket,SOL_SOCKET,SO_REUSEADDR,&r,sizeof(r));


    if (bind(mServerSocket,(struct sockaddr *)&sockadd,sizeof(sockadd))<0){
        Logging::log("Could not bind SMTP server socket\r\n");
        return;
    }

    listen(mServerSocket,1);

    char *buf = new char[BUF_SIZE+1];

    setStarted();
    while (!stopping())
    {
        int clientSocket = accept(mServerSocket,0,0);
        if (clientSocket <0)
        {
            break;
        }
        Logging::log("New SMTP client connection");
        send(clientSocket,"220\r\n",5,0);
        
        int size=0;
        bool finished = false;
        bool receivingData = false;
        char data[10000];
        int n=0;
        while (!finished)
        {
            //TODO: right now we assume that this is comming from alarm system. We should check the From address to make sure of this
            size=read(clientSocket,buf,BUF_SIZE);
      //      Logging::log("SMTP: Got %i bytes",size);
            if (size>0)
            {
                if (!strncmp(buf,"QUIT",4)){
                    send(clientSocket,"221\r\n",5,0);
                    finished = true;
                } else if (!strncmp(buf,"DATA",4)){
                    send(clientSocket,"354 start message\r\n",19,0);
                    Logging::log("SMTP: Getting data");
                    receivingData = true;
                } else {
                    if (receivingData){
                        buf[size]=0;
                        if (size>=5)
                        {
                            strcpy((char*)&data[n],buf);
                            n+=size;
                            if (buf[size-1]=='\n' && buf[size-2]=='\r' &&
                                buf[size-3]=='.' && buf[size-4]=='\n' && buf[size-5]=='\r')
                            {
                                send(clientSocket,"250 OK\r\n",8,0);
                            }
                        } else if (size==3) {
                            // TODO: This could be the end of data
                            if (buf[size-1]=='\n' && buf[size-2]=='\r' && buf[size-3]=='.')
                            {
                                send(clientSocket,"250 OK\r\n",8,0);
                            }
                        }
                    } else {
                        send(clientSocket,"250 OK\r\n",8,0);
                    }
                }
            } else {
                finished = true;
            }
        }

        data[n]=0;
        Logging::log("Closing SMTP client connection");
        close(clientSocket);
        AlarmState as;
        Dumais::JSON::JSON json;
        as.onEmail(json,(char*)&data);
        mpEventProcessor->processEvent(json);

    }
    delete buf;
    if (mServerSocket) close(mServerSocket);

}