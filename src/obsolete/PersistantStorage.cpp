#include "Logging.h"
#include "PersistantStorage.h"
#include <string.h>

PersistantStorage::PersistantStorage(const char* fname)
{
   db_create(&mpDB,0,0);
   mpDB->open(mpDB,0,fname,"",DB_HASH,DB_CREATE,664); 
}

PersistantStorage::~PersistantStorage()
{
    mpDB->close(mpDB,0);
}

std::string PersistantStorage::read(std::string key)
{
    DBT k,data;
    std::string ret = "";
    memset(&k, 0, sizeof(k));
    k.data = (char*)key.c_str(); k.size = key.size();
    if (mpDB->get(mpDB,0,&k,&data,0)==0)
    {
        char tmp[1024];
        if (k.size>1024)
        {
            Logging::log("Persistant Storage: Data too large");
        }
        else
        {
            memcpy(tmp,data.data,data.size); tmp[data.size]=0;
            ret = tmp;
        }
    }
    return ret;
}

void PersistantStorage::write(std::string key, std::string value)
{
    DBT k,data;
    memset(&k, 0, sizeof(k));
    memset(&data, 0, sizeof(data));
    k.data = (char*)key.c_str(); 
    k.size = key.size();
    data.data = (char*)value.c_str(); data.size = value.size()+1;

    mpDB->put(mpDB,0,&k,&data,0);
}

