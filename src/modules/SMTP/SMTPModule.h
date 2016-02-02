#ifndef SMTPSERVER_H
#define SMTPSERVER_H
#include "Module.h"

class SMTPModule: public Module{
private:
    int mServerSocket;
    int mBindPort;

protected:
    virtual void configure(Dumais::JSON::JSON& config);

public:
	SMTPModule();
	~SMTPModule();

    virtual void stop();
    virtual void run();
    virtual std::string getName(){return "smtp";}

    void registerCallBacks(ThreadSafeRestEngine* pEngine);

};

#endif

