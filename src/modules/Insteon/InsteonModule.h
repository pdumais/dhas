#ifndef INSTEON_H
#define INSTEON_H
#include "Module.h"
#include <map>
#include <list>
#include <queue>
#include <string>
#include "InsteonDevice.h"
#include "JSON.h"
#include "IInsteonMessageHandler.h"
#include "InsteonModem.h"

class Insteon: public Module, public IInsteonMessageHandler
{
private:
    std::map<InsteonID,InsteonDevice*> mModules;
    int currentState;
    InsteonID mControllerID;
    InsteonModem *mpInsteonModem;

    void setController(InsteonID id);
    void clearModuleList();
    void listModules(Dumais::JSON::JSON& json);
    InsteonID getControllerID();
    void addModuleDefinition(InsteonID id, std::string name);
    void addEZFlora(InsteonID id, std::string name);

    // switches
    void lightOn(InsteonID id, unsigned char level=0xFF, unsigned char subdev=0);
    void lightOff(InsteonID id, unsigned char subdev);
    void rampOn(InsteonID id, unsigned char level=0x0F, unsigned char rate=0x0F);
    void rampOff(InsteonID id, unsigned char rate=0x0F);
    void lightToggle(InsteonID id, unsigned char subdev);

    // EZFlora
    void setEZFloraProgram(InsteonID id, unsigned char program,
            unsigned char timer1,unsigned char timer2,unsigned char timer3,
            unsigned char timer4,unsigned char timer5,unsigned char timer6,
            unsigned char timer7,unsigned char timer8);

protected:
    virtual void configure(Dumais::JSON::JSON& config);
public:
	Insteon();
	~Insteon();

    virtual void onInsteonMessage(unsigned char *buf);
    virtual void onInsteonAllLinkRecordResponse(unsigned char *buf);
    virtual void onInsteonMessageSent(InsteonID id, IInsteonMessage* cmd);

    void registerCallBacks(RESTEngine* pEngine);

    void startalllinkingasresponder_callback(RESTContext context);
    void sendAllLink_callback(RESTContext context);
    void clearmodules_callback(RESTContext context);
    void listmodules_callback(RESTContext context);
    void addmodule_callback(RESTContext context);
    void addezflora_callback(RESTContext context);
    void addIOLinc_callback(RESTContext context);
    void setcontroller_callback(RESTContext context);
    void ramp_callback(RESTContext context);
    void switch_callback(RESTContext context);
    void ezflorasetprogram_callback(RESTContext context);
    void ezflorastartprogram_callback(RESTContext context);
    void ezflorastopprogram_callback(RESTContext context);
    void ezflorastartvalve_callback(RESTContext context);
    void ezflorastopvalve_callback(RESTContext context);
    void ezfloraforcegetvalvestatus_callback(RESTContext context);
    void raw_callback(RESTContext context);
    void refreshAllLinksDatabase_callback(RESTContext context);
    //void setThermostatOperatingMode_callback(RESTContext context);
    //void setThermostatPoint_callback(RESTContext context);
    //void setThermostatProgramPoint_callback(RESTContext context);
    //void getThermostatProgramPoints_callback(RESTContext context);
    //void removeThermostatProgramPoint_callback(RESTContext context);
    //void activateThermostatProgram_callback(RESTContext context);

    virtual void run();
    virtual std::string getName(){return "insteon";}
    virtual void stop() {}
};


#endif

