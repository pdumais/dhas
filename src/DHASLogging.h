#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include "utils/Logging.h"

class SyslogLogging: public Dumais::Utils::ILogger
{
private:
    std::string syslogServer;
public:
	SyslogLogging(const std::string& server);
	~SyslogLogging();

    virtual void log(const std::string& ss);
};


