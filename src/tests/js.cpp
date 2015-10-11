#include "DHASLogging.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include "JSEngine.h"
#include "config.h"

int main(int argc, char** argv) 
{ 
    Logging::syslog  = false;

    IScriptEngine* p = new JSEngine();
    p->load(argv[1]);
    
    p->notifyEvent("{\"event\":\"test1\"}");

    delete p;

}
