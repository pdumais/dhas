#include <iomanip>
#include "InsteonModem.h"
#include "utils/Logging.h"
#include <unistd.h>
#include "StandardOrExtendedMessage.h"
#include "GroupCleanupMessage.h"
#include "AllLinkDatabaseMessage.h"
#include "IMConfigurationMessage.h"
#include "StartAllLinkingMessage.h"

InsteonModem::InsteonModem(char *serialPort, IInsteonMessageHandler *pHandler)
{
    pthread_mutex_init(&queueLock,0);
    mpSerialPort = new SerialPort(serialPort);
    mTimeLastSend = 0;
    mpInsteonMessageHandler = pHandler;

    // We will receive a 0x15 at the begining. Not sure why but just discard it.
    // NOT TRUE ON x86. IT WAS ONLY ON rPi
    //    while (getByte()!=0x15);

    /* This sucks but it is the only way I could find to make the PLM
       pass the broadcast messages to the application. According to the 
       2412sdevguide.pdf, monitor mode is required because the PLM
        will only pass messages with the PLM's address to the application
    */
    setIMConfiguration(0b01000000); // set in Monitor mode
}

InsteonModem::~InsteonModem()
{
    pthread_mutex_destroy(&queueLock);
    delete mpSerialPort;
    mpSerialPort = 0;
}


bool InsteonModem::messageResponseWaiting()
{
    return mWaitingForResponse;
}

bool InsteonModem::process(bool readyToSend)
{
    // readyToSend is false if one of the device is expected to send an ACK. 
    // mWaitingForResponse is true if we sent a command to the PLM and we are waiting for a response from
    // the PLM (like get first ALL-link record etc...
    if (readyToSend && !mWaitingForResponse)
    {
        if (mInsteonCommandQueue.size()>0)
        {
            pthread_mutex_lock(&queueLock);
            IInsteonMessage* cmd = mInsteonCommandQueue.front();
            mInsteonCommandQueue.pop();
            pthread_mutex_unlock(&queueLock);
            sendCommand(cmd);
            delete cmd;
        }
    }

    int b = getByte();
    if (b>=0)
    {
//        if (b==0x15) return false; // The first byte we get is 0x15. Not sure why but discard it
        if (b==0x02)
        {
            waitForUnsolicitedMessage();
        }
        else 
        {
            LOG("Insteon received unexpected byte: 0x" << std::hex << (unsigned int)b);
        }
    } else {
        return true;
    }

    return false;
}


int InsteonModem::getByte()
{
    //TODO: what is service is trying to stop? will we be stuck in here?
    unsigned char c;
    int size = mpSerialPort->Read(&c,1);
    if (size<1) return -1;

//    Logging::log("0x%x\r\n",c);
    return c; // make sure that bit 7 does not get transfered to bit 31 !!!
}

void InsteonModem::processAllLinkRecordResponse(unsigned char* buf)
{
    mWaitingForResponse = false;
    mpInsteonMessageHandler->onInsteonAllLinkRecordResponse(buf);
    getNextAllLinkRecord();
}


void InsteonModem::waitForUnsolicitedMessage()
{
    // Start of text was already read

    int index = 1;
    bool extended = false;
    unsigned char cmd;
    unsigned char cmd1,cmd2;
    InsteonID to,from;
    
    unsigned char buf[50];
    for (unsigned char i=0;i<50;i++) buf[i]=0;
    int b=-1;
    int length=0;
    while (b>=-1) // because -2 = stopping
    {
        b = getByte();
        if (b>=0)
        {
            if (index==1)
            {
                cmd = b;
                switch (cmd)
                {
                    case 0x50: length=11; break;
                    case 0x51: length=25; break;
                    case 0x52: length=4; break;
                    case 0x53: length=10; break;
                    case 0x54: length=3; break;
                    case 0x55: length=2; break;
                    case 0x56: length=7; break;
                    case 0x57: length=10; break;
                    case 0x58: length=3; break;
                    default:
                    {
                        LOG("Got unexpected byte when waiting for unsolicited message: 0x" << std::hex << (unsigned int)cmd);
                        return;
                    }
                    
                }
            }
            buf[index]=b;
            index++;
            if (index==length)
            {
                switch (cmd)
                {
                    case 0x50: 
                        processStandardMessage((unsigned char*)&buf);            
                        break;
                    case 0x57:
                        processAllLinkRecordResponse((unsigned char*)&buf);
                        break;
                    case 0x51:
                    case 0x52:
                    case 0x53:
                    case 0x54:
                    case 0x55:
                    case 0x56:
                    case 0x58:
                    default:
                        LOG("Insteon unknown command received: " << cmd);
                
                }
                break; // get out of loop
            }
        }
    }
}

bool InsteonModem::waitForReply(IInsteonMessage *cmd)
{
    int b = -1;
    int size=0;
    time_t t,t2;
    time(&t);
    unsigned char buf[50];
    while (b>=-1) // because -2 = stopping
    {
        b = getByte();
        time(&t2);
        // 2seconds have elapsed. Timeout
        if ((t2-t)>=2)
        {
            LOG("Timed out while waiting for Insteon echo");
            return false;
        }

        if (b>=0)
        {
            buf[size]=b;

            size++;
            if (size==cmd->getEchoSize())
            {
                if (b==0x06){
                    return true;
                } else if (b==0x15){
                    return false;
                } else {
                    std::stringstream ss;
                    ss << "Got unknown values while waiting for echo: ";
                    for (int i=0;i<size;i++) ss << "0x" <<std::hex << static_cast<int>(buf[i]) << " ";
                    LOG(ss.str());
                    return false;
                }
            }
        }
    }
    return false;
}

void InsteonModem::processStandardMessage(unsigned char *buf)
{
    InsteonID id = (buf[2]<<16)|(buf[3]<<8)|buf[4];
    std::stringstream ss;
    ss << "RX: 02 ";
    for (int i=0;i<12;i++) ss << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)((unsigned char)buf[i]) << " ";
    LOG(ss.str());

//    Logging::log("Insteon cmd1=0x%x, cmd2=0x%x, flags=0x%x, device=0x%x",buf[9],buf[10],buf[8],id);
    mpInsteonMessageHandler->onInsteonMessage(buf);
}

bool InsteonModem::sendCommand(IInsteonMessage* cmd)
{

    std::string log = "Sending ";
    log+= cmd->toString();
    LOG(log);

    std::stringstream ss;
    ss << "TX: ";
    for (int i=0;i<cmd->getSize();i++) ss << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)(cmd->getBuffer()[i]) <<" ";
    LOG(ss.str());

    mpSerialPort->Write(cmd->getBuffer(),cmd->getSize());

    // We wait for the echo right away
    //TODO: This is not good. The echo could come back asynchronously. Meaning
    //  that an unsolicited event could arrive btween transmission and recption of echo.
    bool ack = waitForReply(cmd);
    if (!ack)
    {
        LOG("ERROR: got Nak from PLM");
        return false;
    }

    InsteonID id = cmd->getDestination();
    if (id==0 && cmd->needAck())
    {
        mWaitingForResponse = true;   
    }
    mpInsteonMessageHandler->onInsteonMessageSent(id,cmd);
    return true;
}

void InsteonModem::getFirstAllLinkRecord()
{

    AllLinkDatabaseMessage* cmd = new AllLinkDatabaseMessage(true);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}

void InsteonModem::getNextAllLinkRecord()
{
    AllLinkDatabaseMessage* cmd = new AllLinkDatabaseMessage(false);

    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}

void InsteonModem::setIMConfiguration(unsigned char imcmd)
{
    IMConfigurationMessage* cmd = new IMConfigurationMessage(imcmd);

    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);

}

void InsteonModem::writeCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2)
{
    StandardOrExtendedMessage *cmd = new StandardOrExtendedMessage(destination,cmd1,cmd2);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}

void InsteonModem::writeI2CSCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char *buf)
{
    unsigned char data[14];
    if (buf!=0)
    {
        for (unsigned char i=0;i<14;i++) data[i] = buf[i];
    }
    else 
    {
        for (unsigned char i=0;i<14;i++) data[i] = 0;
    }
    StandardOrExtendedMessage *cmd = new StandardOrExtendedMessage(destination,cmd1,cmd2,(unsigned char*)&data[0],true);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}

void InsteonModem::startAllLikingAsController(unsigned char group)
{
    StartAllLinkingMessage *cmd = new StartAllLinkingMessage(group,StartAllLinkingMessage::Controller);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}

void InsteonModem::startAllLikingAsResponder(unsigned char group)
{
    StartAllLinkingMessage *cmd = new StartAllLinkingMessage(group,StartAllLinkingMessage::Responder);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}


void InsteonModem::writeGroupCleanupCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2)
{
    unsigned char data[14];
    for (unsigned char i=0;i<14;i++) data[i] = 0;
    GroupCleanupMessage *cmd = new GroupCleanupMessage(destination,cmd1,cmd2);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);
}


void InsteonModem::writeExtendedCommand(InsteonID destination, unsigned char cmd1, unsigned char cmd2, unsigned char* data)
{
    StandardOrExtendedMessage *cmd = new StandardOrExtendedMessage(destination,cmd1,cmd2, data);
    pthread_mutex_lock(&queueLock);
    mInsteonCommandQueue.push(cmd);
    pthread_mutex_unlock(&queueLock);

}
