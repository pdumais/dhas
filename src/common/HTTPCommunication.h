#ifndef HTTPCOMMUNICATION_H
#define HTTPCOMMUNICATION_H
#include "JSON.h"

class HTTPCommunication{
private:

public:
	HTTPCommunication();
	~HTTPCommunication();

    static std::string getURL(std::string server, std::string url, int port=80);
    static bool postURL(std::string server, std::string url, std::string data, std::string contentType, int port=80);

};

#endif

