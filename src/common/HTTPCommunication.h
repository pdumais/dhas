#ifndef HTTPCOMMUNICATION_H
#define HTTPCOMMUNICATION_H
#include "JSON.h"

class HTTPCommunication{
private:

    static bool postOrPutURL(std::string method, std::string server, std::string url, std::string data, std::string contentType, int port=80);
public:
	HTTPCommunication();
	~HTTPCommunication();

    static bool exists(std::string server, std::string url, int port=80);
    static std::string getURL(std::string server, std::string url, int port=80);
    static bool postURL(std::string server, std::string url, std::string data, std::string contentType, int port=80);
    static bool putURL(std::string server, std::string url, std::string data, std::string contentType, int port=80);

};

#endif

