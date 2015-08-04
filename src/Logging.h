#ifndef LOGGING_H
#define LOGGING_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

class Logging{
private:
public:
	Logging();
	~Logging();

    static std::string syslogServer;
    static bool syslog;
    static bool disabled;
    static void log(const char *st,...);
};

#endif

