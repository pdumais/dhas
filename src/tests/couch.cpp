#include "CouchDB.h"
#include "config.h"
#include "Logging.h"
#include <unistd.h>
#include <fstream>
#include <streambuf>

int main(int argc, char** argv)
{
    Logging::syslog = false;

    CouchDB* couch = new CouchDB("dhas2","127.0.0.1",5984);

//    printf("Compacting\r\n");
  //  couch->compact();

    couch->createDb();
    std::ifstream file("views.json");
    std::string st((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    couch->createViewsIfNonExistant(st);

    sleep(1);
    printf("Deleting instance\r\n");
    delete couch;

    
    return 0;
}
