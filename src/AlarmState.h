#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H
#include "json/JSON.h"
#include <list>
#include <string>
#define USER_PAT "System Master"
#define USER_GEN "GEN"

class AlarmState{
private:

public:
	AlarmState();
	~AlarmState();

	void onEmail(Dumais::JSON::JSON& json, char *buffer);
	void getLine(char *buffer, char *key, char *ret);
};

#endif

