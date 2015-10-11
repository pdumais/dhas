#include "../DHASLogging.h"
#include "SerialPort.h"
#include <string.h>
#include <unistd.h>

SerialPort::SerialPort(char *serialPort){
    struct termios newtio;

    mSerialPortHandle  = open(serialPort,O_RDWR | O_NOCTTY | O_NDELAY);
    if (mSerialPortHandle>=0)
    {
        memset(&newtio, 0,sizeof(newtio));
        newtio.c_cflag = B19200 | 0 | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR | IGNPAR;
        newtio.c_oflag = ONLRET;
        newtio.c_lflag = 0;
        tcflush(mSerialPortHandle, TCIFLUSH);
        tcsetattr(mSerialPortHandle,TCSANOW,&newtio);
    }
}

SerialPort::~SerialPort(){
    if (mSerialPortHandle>=0)
    {
        close(mSerialPortHandle);
        mSerialPortHandle=0;
    }
}

int SerialPort::Write(unsigned char *buf, int size)
{
    if (mSerialPortHandle<0) return -1;
    return write(mSerialPortHandle,buf,size);
}

int SerialPort::Read(unsigned char* buf, int maxSize)
{
    if (mSerialPortHandle<0) return -1;
    return read(mSerialPortHandle,buf,maxSize);

}

