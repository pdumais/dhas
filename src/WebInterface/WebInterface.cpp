#include "Logging.h"
#include "WebInterface.h"
#include <string.h>
#include <stdio.h>
#include <list>
#include <sstream>
#include "JSON.h"
#include <stdlib.h>
#include "WebServerLogging.h"

using namespace Dumais::WebServer;

class WebServerInterfaceLogger: public Dumais::WebServer::IWebServerLogger
{
    virtual void log(const std::string& ss)
    {
        Logging::log("%s",ss.c_str());
    }
};


WebInterface::WebInterface(int port,RESTInterface *pRESTInterface, WebNotificationEngine *pWebNotificationEngine)
{
    mpRESTInterface = pRESTInterface;
    Dumais::WebServer::WebServerLogging::logger = new WebServerInterfaceLogger();
//    mpWebNotificationEngine = pWebNotificationEngine;
    mPort = port;
    mpWebServer = 0;
    mpThread = 0;
}

WebInterface::~WebInterface()
{
    if (mpWebServer) delete mpWebServer;
    mpWebServer = 0;
}


void WebInterface::configure(Dumais::JSON::JSON& config)
{
    this->mPasswdFile = config["passwd"].str();
}

void WebInterface::stop()
{
    if (mpWebServer)
    {
        Logging::log("Attempting to shutdown Web server");
        mpWebServer->stop();
    }
    if (mpThread) mpThread->join();
}

void WebInterface::onConnectionOpen()
{
//    Logging::log("WebInterface::onConnectionOpen(%i)",sock);
    if (mpWebServer->getConnectionCount()>10) Logging::log("WARNING: There are %i connections opened on web server",mpWebServer->getConnectionCount());
}

void WebInterface::onConnectionClosed()
{
  //  Logging::log("WebInterface::onConnectionClosed(%i)",sock);
//    mpWebNotificationEngine->unsubscribe(sock);
}

void WebInterface::onResponseSent()
{
//    mpWebNotificationEngine->activateSubscription(sock);
}

HTTPResponse* WebInterface::processHTTPRequest(HTTPRequest* request)
{
    HTTPResponse *resp;

    Dumais::JSON::JSON json;
    std::string method = request->getMethod();
    if (method != "GET")
    {
        resp = HTTPProtocol::buildBufferedResponse(NotImplemented,"","");
    }
    else
    {
        std::string url = request->getURL();
        // We will not support SSE anymore because we don't use it. I removed
        // the code from dumaislib to support persistant connections. There is a backup
        // in a "backup-server-sent-events" folder with that code
        /*if (request->accepts("text/event-stream") && url=="/subscribe")
        {
            mpWebNotificationEngine->subscribe(id);
            resp = HTTPProtocol::buildBufferedResponse(OK,"","text/event-stream","keep-alive","chunked");
        }
        else
        {*/
            std::string responseType = "application/json";
            bool queryProcessed = mpRESTInterface->processQuery(json,url);
            if (queryProcessed)
            {
                resp = HTTPProtocol::buildBufferedResponse(OK,json.stringify(false),responseType);
            }
            else
            {
                resp = HTTPProtocol::buildBufferedResponse(NotFound,"","");
            }   
        //}
    }
    return resp;
}

// This server accepts one connection at a time only. So sockets are blocking
void WebInterface::start()
{
    mpWebServer = new WebServer(mPort, "0.0.0.0",100);
    mpWebServer->requireAuth(this->mPasswdFile.c_str(), 10);
    mpWebServer->setListener(this);
    this->mpWebServer->start();
}

