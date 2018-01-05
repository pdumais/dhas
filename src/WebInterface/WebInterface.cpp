#include "DHASLogging.h"
#include "WebInterface.h"
#include <string.h>
#include <stdio.h>
#include <list>
#include <sstream>
#include "json/JSON.h"
#include <stdlib.h>

using namespace Dumais::WebServer;

WebInterface::WebInterface(int port,RESTInterface *pRESTInterface, WebNotificationEngine *pWebNotificationEngine)
{
    mpRESTInterface = pRESTInterface;
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
    this->mSSLCert = config["ssl_cert"].str();
    this->mSSLKey = config["ssl_key"].str();
}

void WebInterface::stop()
{
    mStopped = true;
    if (mpWebServer)
    {
        LOG("Attempting to shutdown Web server");
        mpWebServer->stop();
    }
    if (mpThread) mpThread->join();
}

void WebInterface::onConnectionOpen()
{
//    Logging::log("WebInterface::onConnectionOpen(%i)",sock);
    if (mpWebServer->getConnectionCount()>10) LOG("WARNING: There are "<< mpWebServer->getConnectionCount()<<" connections opened on web server");
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

void WebInterface::processHTTPRequestAsync(Dumais::WebServer::HTTPRequest* req, Dumais::WebServer::HTTPRequestCallBack cb)
{
}

// This server accepts one connection at a time only. So sockets are blocking
void WebInterface::start()
{
    mStopped = false;
    mpWebServer = new WebServer(mPort, "0.0.0.0",100);
    mpWebServer->requireAuth(this->mPasswdFile.c_str(), 10);
    mpWebServer->setListener(this);
    mpWebServer->setStopEventHandler([this](){
        if (!mStopped)
        {
            LOG("ERROR: The webserver closed unexpectedly");
            // TODO: we should be able to send an email out in that case
        }
    });
    //this->mpWebServer->start();
    this->mpWebServer->startSecure(this->mSSLCert.c_str(),this->mSSLKey.c_str());
}

