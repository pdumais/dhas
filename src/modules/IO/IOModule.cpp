#include "IOModule.h"
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
#include <math.h>
#include "HTTPCommunication.h"
#include "regex.h"
#include <string>
#include "UDev.h"
#include "ModuleRegistrar.h"

#define BUF_SIZE 1024

REGISTER_MODULE(IOModule)

IOModule::IOModule()
{
    mServer = "";
    std::string port = UDev::findDevice("03eb","204b");
    if (port == "")
    {
        Logging::log("Could not find an IO board device");
        mpSerialPort = 0;
    }
    else
    {
        Logging::log("IO board device found at %s",port.c_str());
        mpSerialPort = new SerialPort(port.c_str());
    }
    mCurrentInputStatus = 0;
}

IOModule::~IOModule(){
}

void IOModule::configure(Dumais::JSON::JSON& config)
{
}

void IOModule::stop()
{
    Logging::log("Attempting to shutdown IO Module");
}


void IOModule::registerCallBacks(RESTEngine* pEngine)
{
    RESTCallBack *p;
    p = new RESTCallBack(this,&IOModule::getStatus_callback,"Get current status of sensors");
    pEngine->addCallBack("/io/getstatus","GET",p);

    p = new RESTCallBack(this,&IOModule::addWebRelay_callback,"add web relay device");
    p->addParam("ip","IP address of a web relay device");
    p->addParam("id","ID of a web relay on the device");
    p->addParam("name","Name of relay. This will be used to trigger the relay later");
    pEngine->addCallBack("/io/addwebrelay","GET",p);

    p = new RESTCallBack(this,&IOModule::triggerRelay_callback,"Set relay on or off");
    p->addParam("on","'true' or 'false'");
    p->addParam("name","Name of relay. This will be used to trigger the relay later");
    pEngine->addCallBack("/io/setrelay","GET",p);

    p = new RESTCallBack(this,&IOModule::triggerIORelay_callback,"Set IO board output pin on or off");
    p->addParam("on","'true' or 'false'");
    p->addParam("number","pin number");
    pEngine->addCallBack("/io/setoutput","GET",p);
}

void IOModule::run()
{
    unsigned char b;
    setStarted();
    while (!stopping())
    {
        if (mpSerialPort)
        {
            mSerialPortLock.lock();
            int ret = mpSerialPort->Read(&b,1);
            if (ret == 1)
            {
                mCurrentInputStatus = b;
                Dumais::JSON::JSON json;
                this->statusToJSON(json);
                mpEventProcessor->processEvent(json);
            }

            //For some reason, read() will not return -1 if the device was disconnected.
            // So we need to try to do a write to check for an error
            if (mpSerialPort->Write("\0",1)<1)
            {
                Logging::log("Serial port error");
                delete mpSerialPort;
                mpSerialPort = 0;
            }
            mSerialPortLock.unlock();
        }
        else
        {
            std::string port = UDev::findDevice("03eb","204b");
            if (port != "")
            {       
                Logging::log("new IO board device was found at %s",port.c_str());
                mpSerialPort = new SerialPort(port.c_str());
            }
        }
        usleep(250000);
    }

}

void IOModule::statusToJSON(Dumais::JSON::JSON &json)
{
    json.addValue("inputpin","event");
    json.addList("pins");
    for (int i = 0; i < 8; i++) json["pins"].addValue(((mCurrentInputStatus>>i)&1));
}

void IOModule::getStatus_callback(RESTContext context)
{
    RESTParameters* params = context.params;
    Dumais::JSON::JSON& json = context.returnData;
    bool ok = true;
    mSerialPortLock.lock();
    if (mpSerialPort)
    {
        mpSerialPort->Write("?",1);
        time_t t,t2;
        time(&t);
        while (mpSerialPort->Read(&mCurrentInputStatus,1) != 1)
        {
            time(&t2);
            if (t2 > (t+2)) 
            {
                ok = false;
                break;
            }
        }
    }
    else
    {
        ok = false;
    }
    mSerialPortLock.unlock();
    if (ok)
    {
        this->statusToJSON(json);
    }
    else
    {
        json.addValue("Time out","Error");
    }
}

void IOModule::triggerRelay_callback(RESTContext context)
{
    RESTParameters* params = context.params;
    Dumais::JSON::JSON& json = context.returnData;
    std::string val = params->getParam("on");
    std::string name = params->getParam("name");
    
    if ((val == "true" || val == "false") && mWebRelayList.count(name) == 1)
    {
        if (val=="true") val = "1"; else val = "0";
        std::string url = "/state.xml?relay"+mWebRelayList[name].id+"State="+val;
        std::string st = HTTPCommunication::getURL(mWebRelayList[name].ip,url); 
        json.addValue("ok","status");
        return;
    }

    json.addValue("error","status");
}


void IOModule::triggerIORelay_callback(RESTContext context)
{
    RESTParameters* params = context.params;
    Dumais::JSON::JSON& json = context.returnData;
    std::string val = params->getParam("on");
    std::string str = params->getParam("number");
    unsigned long number = std::stoul(str);

    if ((val == "true" || val == "false") && number >=0 && number <8)
    {
        unsigned char b;
        if (val=="true") b='a'+number; else b='A'+number;
        mSerialPortLock.lock();
        if (mpSerialPort)
        {
            Logging::log("IO: Writing '%c' to serial port",b);
            mpSerialPort->Write(&b,1);
            json.addValue("ok","status");
        }
        else
        {
            json.addValue("error","status");
        }
        mSerialPortLock.unlock();
        return;
    }
    else
    {
        Logging::log("IO service invalid IOrelay: %s, %x",val.c_str(),number);
    }

    json.addValue("error","status");
}


void IOModule::addWebRelay_callback(RESTContext context)
{
    RESTParameters* params = context.params;
    Dumais::JSON::JSON& json = context.returnData;
    WebRelay r;
    r.ip = params->getParam("ip");
    r.id = params->getParam("id");
    mWebRelayList[params->getParam("name")] = r;
    Logging::log("IO: Webrelay added: %s:%s",r.ip.c_str(),r.id.c_str());
    json.addValue("ok","status");
}

