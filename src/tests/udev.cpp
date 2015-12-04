#include "DHASLogging.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include "config.h"
#include "UDev.h"
#include <string.h>
    
unsigned long getMemUsage()
{ 
    char buf[1024];

    FILE* f = fopen("/proc/self/status","r");
    while (fgets(buf,1024,f))
    {
        if (!strncmp(buf,"VmSize:",7))
        {
            fclose(f);
            return atoi((char*)&buf[8]);
        }
    }
    fclose(f);
    return 0;
}


int main(int argc, char** argv) 
{ 
    unsigned int i;
    printf("mem use before: %x\r\n",getMemUsage());
    for (i = 0; i < 1000; i++)
    {
        std::string st = UDev::findDevice("13eb","104b");
    }
    printf("mem use After: %x\r\n",getMemUsage());
}
