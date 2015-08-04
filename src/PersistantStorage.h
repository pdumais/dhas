#ifndef PERSISTANTSTORAGE_H
#define PERSISTANTSTORAGE_H

#include <string>
#include <stdio.h>
#define HAVE_CXX_STDHEADERS
#include <db_cxx.h>

class PersistantStorage{
private:
    DB *mpDB;
public:
	PersistantStorage(const char* fname);
	~PersistantStorage();

    std::string read(std::string key);

    void write(std::string key, std::string value);

};

#endif

