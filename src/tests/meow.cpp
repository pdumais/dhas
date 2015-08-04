#include "CouchDB.h"
#include "config.h"
#include "Logging.h"
#include <unistd.h>

int main(int argc, char** argv)
{
    Logging::syslog = false;

    CouchDB* couch = new CouchDB("dhas");

    printf("Compacting\r\n");
    couch->compact();

    sleep(2);
    printf("Deleting instance\r\n");
    delete couch;

    
    return 0;
}
