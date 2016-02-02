#include "DHASLogging.h"
#include "InsteonModule.h"
#include "SwitchDevice.h"
#include "EZFloraDevice.h"
#include "IOLinc.h"
#include "Thermostat.h"
#include "KeypadLinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include "StandardOrExtendedMessage.h"
#include "ModuleRegistrar.h"

#define MAX_READ_SIZE 1024

REGISTER_MODULE(Insteon)

/*
TODO: insteon API should be able to block until ACK/NAK/timeout

InsteonModule cannot block to wait for ACK/NAK after sending a response because the
RESTInterface only allows 1 command at a time because of a mutex. If the service blocks, this will
block the whole RESTInterface. Also, the service just pools a command, so it cannot wait
for mWaitingResponse of InsteonModem, because at that time, the command might not even have been 
sent to the modem. The webservice cannot have the responsibility of blocking since it doesn't know if this was an Insteon command.
the Insteon service is the only one that could block.

even if we would like to block in order for webpage to display proper status, this would not work 
since after a device has changed state, it might have triggered another device to change and we would not
know about it. Manually refreshing the UI seems innevitable
*/

Insteon::Insteon()
{
}

Insteon::~Insteon()
{
    delete mpInsteonModem;
}

void Insteon::configure(Dumais::JSON::JSON& config)
{
    mpInsteonModem = new InsteonModem(config["serialport"].str().c_str(),this);
}

void Insteon::registerCallBacks(ThreadSafeRestEngine* pEngine)
{
    RESTCallBack* p;
//    p = new RESTCallBack<Insteon>(this,&Insteon::startalllinkingasresponder_callback,"");
//    pEngine->addCallBack("/insteon/startalllinkingasresponder",p);
//    p = new RESTCallBack<Insteon>(this,&Insteon::sendAllLink_callback,"");
//    pEngine->addCallBack("/insteon/sendalllink",p);
//    p = new RESTCallBack<Insteon>(this,&Insteon::ramp_callback,"");
//    pEngine->addCallBack("/insteon/ramp",p);

    p = new RESTCallBack(this,&Insteon::clearmodules_callback,"reset list of Insteon module definition");
    pEngine->addCallBack("/insteon/clearmodules","GET",p);

    p = new RESTCallBack(this,&Insteon::listmodules_callback,"lists all Insteon modules");
    pEngine->addCallBack("/insteon/listmodules","GET",p);

    p = new RESTCallBack(this,&Insteon::addmodule_callback,"add a Insteon module definition");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("name","The name of the module");
    pEngine->addCallBack("/insteon/addmodule","GET",p);

    p = new RESTCallBack(this,&Insteon::addezflora_callback,"add a Insteon ezflora module definition");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("name","The name of the module");
    pEngine->addCallBack("/insteon/addezflora","GET",p);

    p = new RESTCallBack(this,&Insteon::addIOLinc_callback,"add a Insteon iolinc module definition");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("name","The name of the module");
    pEngine->addCallBack("/insteon/addiolinc","GET",p);

    /*p = new RESTCallBack<Insteon>(this,&Insteon::addKeypadLinc_callback,"add a Insteon keypadlinc module definition");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("name","The name of the module");
    pEngine->addCallBack("/insteon/addkeypadlinc",p);*/

    /*p = new RESTCallBack<Insteon>(this,&Insteon::addThermostat_callback,"add a Insteon thermostat module definition");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("name","The name of the module");
    pEngine->addCallBack("/insteon/addthermostat",p);*/

    p = new RESTCallBack(this,&Insteon::setcontroller_callback,"set Insteon controller ID (PLM)");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    pEngine->addCallBack("/insteon/setcontroller","GET",p);

    p = new RESTCallBack(this,&Insteon::switch_callback,"Turn on or off a device");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("action","on/off/toggle");
    p->addParam("level","0 to 255. Irrelevant if action is off or toggle");
    p->addParam("rate","0 to 255. This is the ramp rate");
    p->addParam("subdev","for EZFlora, subdev is 1-7 for valves and 8-11 for programs 1-4. For switches, this is irrelevant");
    pEngine->addCallBack("/insteon/switch","GET",p);

    p = new RESTCallBack(this,&Insteon::ezflorasetprogram_callback,"Sets a program on the EZFlora");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("p","Program number. 1 to 4");
    p->addParam("z1","Zone 1 timer. 0 to 255 minutes");
    p->addParam("z2","Zone 2 timer. 0 to 255 minutes");
    p->addParam("z3","Zone 3 timer. 0 to 255 minutes");
    p->addParam("z4","Zone 4 timer. 0 to 255 minutes");
    p->addParam("z5","Zone 5 timer. 0 to 255 minutes");  
    p->addParam("z6","Zone 6 timer. 0 to 255 minutes");
    p->addParam("z7","Zone 7 timer. 0 to 255 minutes");
    p->addParam("z8","Zone 8 timer. 0 to 255 minutes");
    pEngine->addCallBack("/insteon/ezflora/setprogram","GET",p);

    p = new RESTCallBack(this,&Insteon::ezfloraforcegetvalvestatus_callback,"Forces an update of EZFlora status");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    pEngine->addCallBack("/insteon/ezflora/status","GET",p);

    p = new RESTCallBack(this,&Insteon::raw_callback,"Send raw insteon command");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("cmd1","command byte 1");
    p->addParam("cmd2","command byte 2");
    p->addParam("d1","data byte 1 for extended data");
    p->addParam("d2","data byte 2 for extended data");
    p->addParam("d3","data byte 3 for extended data");
    p->addParam("d4","data byte 4 for extended data");
    p->addParam("d5","data byte 5 for extended data");
    p->addParam("d6","data byte 6 for extended data");
    p->addParam("d7","data byte 7 for extended data");
    p->addParam("d8","data byte 8 for extended data");
    p->addParam("d9","data byte 9 for extended data");
    p->addParam("d10","data byte 10 for extended data");
    p->addParam("d11","data byte 11 for extended data");
    p->addParam("d12","data byte 12 for extended data");
    p->addParam("d13","data byte 13 for extended data");
    p->addParam("d14","data byte 14 for extended data");
    pEngine->addCallBack("/insteon/raw","GET",p);

    p = new RESTCallBack(this,&Insteon::refreshAllLinksDatabase_callback,"retrieve all-link database");
    pEngine->addCallBack("/insteon/refreshalllinksdatabase","GET",p);

    /*p = new RESTCallBack<Insteon>(this,&Insteon::setThermostatOperatingMode_callback,"sets thermostat in cool,heat or off mode");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("mode","off/heat/cool/auto/manualfan");
    pEngine->addCallBack("/insteon/thermostat/setoperatingmode",p);

    p = new RESTCallBack<Insteon>(this,&Insteon::setThermostatPoint_callback,"Sets thermostat setpoint");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("value","temperature in celcius");
    p->addParam("type","heat/cool. The mode for which the setpoint must be set. Note that this does not change the mode.");
    pEngine->addCallBack("/insteon/thermostat/setpoint",p);

    p = new RESTCallBack<Insteon>(this,&Insteon::setThermostatProgramPoint_callback,"Set a thermostat program point");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("t","temperature in celcius");
    p->addParam("hour","hour of the day (0-23)");
    p->addParam("weekday","day of the week (1-7");
    p->addParam("mode","heat/cool. If current operating mode is heat, cool program points will be ignored and vice-versa");
    pEngine->addCallBack("/insteon/thermostat/setprogrampoint",p);

    p = new RESTCallBack<Insteon>(this,&Insteon::getThermostatProgramPoints_callback,"Get all thermostat program points");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    pEngine->addCallBack("/insteon/thermostat/getprogrampoints",p);

    p = new RESTCallBack<Insteon>(this,&Insteon::removeThermostatProgramPoint_callback,"Remove a thermostat program point");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("hour","hour of the day (0-23)");
    p->addParam("weekday","day of the week (1-7");
    pEngine->addCallBack("/insteon/thermostat/removeprogrampoint",p);

    p = new RESTCallBack<Insteon>(this,&Insteon::activateThermostatProgram_callback,"Active thermostat program");
    p->addParam("id","the Insteon device ID formated as 0xNNNNNN");
    p->addParam("active","true if program is to be enabled, false if it is to be disabled.");
    pEngine->addCallBack("/insteon/thermostat/activateprogram",p);*/
    
}


void Insteon::run()
{
    setStarted();
    while (!stopping())
    {
        // Make sure every devices are idle before atempting to send another command
        bool devicesIdle = true;
        for (std::map<InsteonID,InsteonDevice*>::iterator it = mModules.begin();it!=mModules.end();it++)
        {
            time_t t;
            time(&t);
            it->second->onTimer(t);
            if (!it->second->isIdle())
            {
                devicesIdle = false;
                break;
            }
        }

        if (mpInsteonModem->process(devicesIdle))
        {
            // dont sleep more than 50ms because Insteon modem expects reponses within 85ms
            usleep(50000);
        }

    }
}

void Insteon::refreshAllLinksDatabase_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    mpInsteonModem->getFirstAllLinkRecord();
    json.addValue("ok","status");
}


void Insteon::clearmodules_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    clearModuleList();
    json.addValue("ok","status");
}

void Insteon::listmodules_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    json.addList("modules");
    listModules(json["modules"]);
}

/*void Insteon::startalllinkingascontroller_callback(RESTContext context)
{
    unsigned int group = strtoul(params->getParam("group").c_str(),0,16);
    mpInsteonModem->startAllLikingAsController(group);
    json.addValue("ok","status");
}*/

void Insteon::startalllinkingasresponder_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    unsigned int group = strtoul(params->getParam("group").c_str(),0,16);
    mpInsteonModem->startAllLikingAsResponder(group);
    json.addValue("ok","status");
}


void Insteon::addmodule_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    if (params->getParam("id")!="""" && params->getParam("name")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        addModuleDefinition(id,params->getParam("name"));
        json.addValue("ok","status");
    }
}

void Insteon::addezflora_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    if (params->getParam("id")!="" && params->getParam("name")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        addEZFlora(id,params->getParam("name"));
        json.addValue("ok","status");
    }
}


void Insteon::addIOLinc_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
   if (params->getParam("id")!="" && params->getParam("name")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        IOLinc *dev = new IOLinc(params->getParam("name"),id,mpInsteonModem);
        mModules[id] = dev;
        json.addValue("ok","status");
    }
}

/*void Insteon::addKeypadLinc_callback(RESTContext context)
{
   if (params->getParam("id")!="" && params->getParam("name")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        KeypadLinc *dev = new KeypadLinc(params->getParam("name"),id,mpInsteonModem);
        mModules[id] = dev;
        json.addValue("ok","status");
    }
}*/


/*void Insteon::addThermostat_callback(RESTContext context)
{
   if (params->getParam("id")!="" && params->getParam("name")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        Thermostat *dev = new Thermostat(params->getParam("name"),id,mpInsteonModem);
        mModules[id] = dev;
        json.addValue("ok","status");
    }
}*/

/*void Insteon::setThermostatOperatingMode_callback(RESTContext context)
{
   if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
        if (it!=mModules.end())
        {
            Thermostat *p = (Thermostat*)it->second;
            Thermostat::ThermostatMode t;
            if (params->getParam("mode")=="off")
            {
                t = Thermostat::Off;
            } else  if (params->getParam("mode")=="heat")
            {
                t = Thermostat::Heat;
            } else  if (params->getParam("mode")=="cool")
            {
                t = Thermostat::Cool;
            } else  if (params->getParam("mode")=="auto")
            {
                t = Thermostat::Auto;
            } else  if (params->getParam("mode")=="manualfan")
            {
                t = Thermostat::ManualFan;
            }
            p->setOperatingMode(t);
        }
        json.addValue("ok","status");
    }
}

void Insteon::setThermostatProgramPoint_callback(RESTContext context)
{
    if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
        if (it!=mModules.end())
        {
            Thermostat *p = (Thermostat*)it->second;
            unsigned char temp = strtoul(params->getParam("t").c_str(),0,10);
            unsigned char hour = strtoul(params->getParam("hour").c_str(),0,10);
            unsigned char weekday = strtoul(params->getParam("weekday").c_str(),0,10);
            Thermostat::ThermostatMode mode;
            if (params->getParam("mode")=="heat")
            {
                mode = Thermostat::Heat;
            } else  if (params->getParam("mode")=="cool") {
                mode = Thermostat::Cool;
            }

            p->setProgramPoint(temp,mode,hour,weekday);

        }
    }
}

void Insteon::removeThermostatProgramPoint_callback(RESTContext context)
{
    if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
        if (it!=mModules.end())
        {
            Thermostat *p = (Thermostat*)it->second;
            unsigned char hour = strtoul(params->getParam("hour").c_str(),0,10);
            unsigned char weekday = strtoul(params->getParam("weekday").c_str(),0,10);
            p->removeProgramPoint(hour,weekday);
        }
    }
}


void Insteon::getThermostatProgramPoints_callback(RESTContext context)
{
    if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
        if (it!=mModules.end())
        {
            Thermostat *p = (Thermostat*)it->second;
            p->getProgramPoints(json);
        }
    }
}

void Insteon::activateThermostatProgram_callback(RESTContext context)
{
    if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
        if (it!=mModules.end())
        {
            bool b = false;
            if (params->getParam("active") == "true") b = true;
            Thermostat *p = (Thermostat*)it->second;
            p->activateProgram(b);
        }
    }
}


void Insteon::setThermostatPoint_callback(RESTContext context)
{
    if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        unsigned char temp = strtoul(params->getParam("value").c_str(),0,10);
        std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
        if (it!=mModules.end())
        {
            Thermostat *p = (Thermostat*)it->second;
            if (params->getParam("type")=="heat")
            {
                p->setHeatPoint(temp);
            }
            if (params->getParam("type")=="cool")
            {
                p->setCoolPoint(temp);
            }
        }
    }
}*/

void Insteon::setcontroller_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    if (params->getParam("id")!="")
    {
        InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
        setController(id);
        json.addValue("ok","status");
    }
}

void Insteon::ramp_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    unsigned char rate = 0x0F;
    unsigned char level = 0x0F;
    if (params->getParam("rate")!="") rate = atoi(params->getParam("rate").c_str());
    if (params->getParam("level")!="") level = atoi(params->getParam("level").c_str());
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    if (params->getParam("action")=="on"){
        rampOn(id,level,rate);
    } else if (params->getParam("action")=="off"){
        rampOff(id,rate);
    }
}

void Insteon::switch_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned char subdev = 0;
    if (params->getParam("subdev")!="")
    {
        subdev = strtoul(params->getParam("subdev").c_str(),0,10);
    }

    if (params->getParam("action")=="on"){
        unsigned char level =255;
        if (params->getParam("level")!="") level = atoi(params->getParam("level").c_str());
        if (params->getParam("rate")!="") level = atoi(params->getParam("rate").c_str());
        lightOn(id,level,subdev);
    } else if (params->getParam("action")=="off"){
        lightOff(id,subdev);
    } else if (params->getParam("action")=="toggle"){
        lightToggle(id,subdev);    
    }
    json.addValue("ok","status");
}

void Insteon::ezflorasetprogram_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned char program = atoi(params->getParam("p").c_str());
    if (program>=0 && program<=4)
    {
        unsigned char z1 = (params->getParam("z1")=="")?0:atoi(params->getParam("z1").c_str());
        unsigned char z2 = (params->getParam("z2")=="")?0:atoi(params->getParam("z2").c_str());
        unsigned char z3 = (params->getParam("z3")=="")?0:atoi(params->getParam("z3").c_str());
        unsigned char z4 = (params->getParam("z4")=="")?0:atoi(params->getParam("z4").c_str());
        unsigned char z5 = (params->getParam("z5")=="")?0:atoi(params->getParam("z5").c_str());
        unsigned char z6 = (params->getParam("z6")=="")?0:atoi(params->getParam("z6").c_str());
        unsigned char z7 = (params->getParam("z7")=="")?0:atoi(params->getParam("z7").c_str());
        unsigned char z8 = (params->getParam("z8")=="")?0:atoi(params->getParam("z8").c_str());
        setEZFloraProgram(id,program,z1,z2,z3,z4,z5,z6,z7,z8);
        json.addValue("ok","status");
    } else {
        json.addValue("Bad Program","status");
    }
}

/*void Insteon::ezflorastartprogram_callback(RESTContext context)
{
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned char program = atoi(params->getParam("p").c_str());
    if (program>0 && program<=4)
    {
        startProgram(id,program);
        json.addValue("ok","status");
    } else {
        json.addValue("Bad Program","status");
    }
}

void Insteon::ezflorastopprogram_callback(RESTContext context)
{
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned char program = atoi(params->getParam("p").c_str());
    if (program>0 && program<=4)
    {
        stopProgram(id,program);
        json.addValue("ok","status");
    } else {
        json.addValue("Bad Program","status");
    }
}

void Insteon::ezflorastartvalve_callback(RESTContext context)
{
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned char valve = atoi(params->getParam("v").c_str());
    if (valve>=0 && valve<=7)
    {
        startValve(id,valve);
        json.addValue("ok","status");
    } else {
        json.addValue("Bad valve number","status");
    }
}

void Insteon::ezflorastopvalve_callback(RESTContext context)
{
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned char valve = atoi(params->getParam("v").c_str());
    if (valve>=0 && valve<=7)
    {
        stopValve(id,valve);
        json.addValue("ok","status");
    } else {
        json.addValue("Bad valve number","status");
    }
}*/

void Insteon::ezfloraforcegetvalvestatus_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    mpInsteonModem->writeCommand(id,0x44,0x02);
    json.addValue("ok","status");
}


void Insteon::raw_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned int cmd1 = strtoul(params->getParam("cmd1").c_str(),0,16);
    unsigned int cmd2 = strtoul(params->getParam("cmd2").c_str(),0,16);

    unsigned char buf[14];
    buf[0] = strtoul(params->getParam("d1").c_str(),0,16);
    buf[1] = strtoul(params->getParam("d2").c_str(),0,16);
    buf[2] = strtoul(params->getParam("d3").c_str(),0,16);
    buf[3] = strtoul(params->getParam("d4").c_str(),0,16);
    buf[4] = strtoul(params->getParam("d5").c_str(),0,16);
    buf[5] = strtoul(params->getParam("d6").c_str(),0,16);
    buf[6] = strtoul(params->getParam("d7").c_str(),0,16);
    buf[7] = strtoul(params->getParam("d8").c_str(),0,16);
    buf[8] = strtoul(params->getParam("d9").c_str(),0,16);
    buf[9] = strtoul(params->getParam("d10").c_str(),0,16);
    buf[10] = strtoul(params->getParam("d11").c_str(),0,16);
    buf[11] = strtoul(params->getParam("d12").c_str(),0,16);
    buf[12] = strtoul(params->getParam("d13").c_str(),0,16);
    buf[13] = strtoul(params->getParam("d14").c_str(),0,16);

    mpInsteonModem->writeI2CSCommand(id,cmd1,cmd2,(unsigned char*)&buf[0]);
    json.addValue("ok","status");
}

void Insteon::sendAllLink_callback(RESTContext* context)
{
    RESTParameters* params = context->params;
    Dumais::JSON::JSON& json = context->returnData;
    InsteonID id = strtoul(params->getParam("id").c_str(),0,16);
    unsigned int group = strtoul(params->getParam("group").c_str(),0,16);
    mpInsteonModem->writeCommand(id,0x09,group);
    json.addValue("ok","status");
}


void Insteon::onInsteonAllLinkRecordResponse(unsigned char *buf)
{
    InsteonID id = (buf[4]<<16)|(buf[5]<<8)|buf[6];
    // check if we know that device
    std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
    if (it==mModules.end()) return;

    LOG("All-Link record response: "<<std::hex<<buf[4]<<buf[5]<<buf[6]<<" flags="<<buf[2]<<", group="<<buf[3]<<", d1="<<buf[7]<<", d2="<<buf[8]<<", d3="<<buf[9]);
    mModules[id]->setGroup(buf[3],buf[7],buf[8],buf[9]);

}

void Insteon::onInsteonMessage(unsigned char *buf)
{
    InsteonID id = (buf[2]<<16)|(buf[3]<<8)|buf[4];
    // check if we know that device
    std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
    if (it==mModules.end()) return;

    Dumais::JSON::JSON json;
    mModules[id]->onInsteonMessage(json,buf);
    if (json["event"].str()!="{invalid}") mpEventProcessor->processEvent(json);
}

void Insteon::onInsteonMessageSent(InsteonID id, IInsteonMessage* cmd)
{
    std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
    if (it==mModules.end())
    {
        LOG("Insteon::onInsteonMessageSent ID not found");
        return;
    }
    
    StandardOrExtendedMessage *pMsg = dynamic_cast<StandardOrExtendedMessage*>(cmd);
    if (pMsg)
    {
        InsteonDirectMessage msg;
        msg.extended = pMsg->isExtended();
        msg.cmd1 = pMsg->command1();
        msg.cmd2 = pMsg->command2();
        if (msg.extended)
        {
            pMsg->copyData((char*)&msg.data[0]);
        }
        mModules[id]->setLastDirectMessageSent(msg);
    }
}

void Insteon::addModuleDefinition(InsteonID id, std::string name)
{
    SwitchDevice *dev = new SwitchDevice(name,id,mpInsteonModem);
    mModules[id] = dev;
}

void Insteon::addEZFlora(InsteonID id, std::string name)
{
    EZFloraDevice *dev = new EZFloraDevice(name,id,mpInsteonModem);
    mModules[id] = dev;
}


void Insteon::setController(InsteonID id)
{
    mControllerID = id;
}

void Insteon::clearModuleList()
{
    /* It is safe to delete all modules since only one service call at a time can be made in the system
     * This is guaranteed by the lock in the RESTInterface. Only the RESTInterface can make service calls.
    */
    this->suspend(); // suspend the Insteon thread so we dont access these devices
    for (std::map<InsteonID,InsteonDevice*>::iterator it=mModules.begin();it!=mModules.end();it++)
    {
        delete it->second;
    }
    mModules.clear();
    this->resume();
}

void Insteon::listModules(Dumais::JSON::JSON& json)
{

    for (std::map<InsteonID,InsteonDevice*>::iterator it=mModules.begin();it!=mModules.end();it++)
    {
        Dumais::JSON::JSON& obj = json.addObject("module");
        it->second->toJSON(obj);
//        printf("TEST %s\r\n",obj.stringify(false).c_str());
    }
}

InsteonID Insteon::getControllerID()
{
    return mControllerID;
}


void Insteon::lightOn(InsteonID id,unsigned char level,unsigned char subdev)
{
    // we want to avoid turning off the light using a light on command
    if (level<10) level = 10;

    std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
    if (it!=mModules.end())
    {
        it->second->turnOnOrOff(true,level,subdev);
    } 
    else 
    {
    }


}

void Insteon::lightOff(InsteonID id,unsigned char subdev)
{
    std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
    if (it!=mModules.end())
    {
        it->second->turnOnOrOff(false,0,subdev);
    }
}

void Insteon::lightToggle(InsteonID id, unsigned char subdev)
{
    std::map<InsteonID,InsteonDevice*>::iterator it = mModules.find(id);
    if (it!=mModules.end())
    {
        it->second->toggle(subdev);
    }
}

void Insteon::rampOn(InsteonID id, unsigned char level, unsigned char rate)
{
    //TODO: the "InsteonDevice" should send it
    mpInsteonModem->writeCommand(id,0x2E,level<<4&(rate&0x0F));
}

void Insteon::rampOff(InsteonID id, unsigned char rate)
{
    //TODO: the "InsteonDevice" should send it
    mpInsteonModem->writeCommand(id,0x2F,(rate&0x0F));
}


//Note: program 0 is default/manual timers. Programs are 1 to 4
void Insteon::setEZFloraProgram(InsteonID id, unsigned char program,unsigned char timer1,
            unsigned char timer2,unsigned char timer3,unsigned char timer4,unsigned char timer5,
            unsigned char timer6,unsigned char timer7,unsigned char timer8)

{
    unsigned char data[14];
    data[0] = timer1;
    data[1] = timer2;
    data[2] = timer3;
    data[3] = timer4;
    data[4] = timer5;
    data[5] = timer6;
    data[6] = timer7;
    data[7] = timer8;

    //TODO: the "InsteonDevice" should send it
    mpInsteonModem->writeExtendedCommand(id,0x40,program,(unsigned char*)&data);
}

