#include "DHASLogging.h"
#include "AlarmState.h"
#include "regex.h"
#include "string.h"
#include "stdio.h"
#include <errno.h>

AlarmState::AlarmState(){
}

AlarmState::~AlarmState(){
}

void AlarmState::getLine(char *buffer, char *key, char *ret)
{
	regex_t preg;
	char *lookup = new char[500];
	sprintf(lookup,"%s *([0-9a-zA-Z :-]*)",key);
        regcomp(&preg,lookup,REG_EXTENDED|REG_ICASE);

	regmatch_t matches[3];
        int err = regexec(&preg,buffer,2,matches,0);
        if (err != REG_NOMATCH){
                int s1 = matches[1].rm_so;
                int e1 = matches[1].rm_eo;
                int size1 = e1-s1;

                memcpy(ret,(char*)&buffer[s1],size1);
		ret[size1] = 0;
        }

	delete lookup;
}

void AlarmState::onEmail(Dumais::JSON::JSON& json, char* buffer)
{
	char status[100];
	char from[100];
	char date[100];

	status[0]=0;
	from[0]=0;
	date[0]=0;

	this->getLine(buffer,"Message:",(char*)&status);
	this->getLine(buffer,"By:",(char*)&from);
	this->getLine(buffer,"Time:",(char*)&date);


	LOG("Alarm: New status: "<< (char*)&status <<" initiated by "<<(char*)&from);
	char query[1000];

	char *wav = "";
	char *mailbox ="";
	bool validUser = true;//false;
	/*if (!strcmp((char*)&from,USER_PAT)){
		wav = "patrick";	
		mailbox = "101";
		validUser = true;
	} else if (!strcmp((char*)&from,USER_GEN)){
		wav = "genevieve";
		mailbox = "102";
		validUser = true;
	} */

    if (validUser)
    {
        time_t t;
        time(&t);
        std::string st = ctime(&t);
        st.resize(st.size()-1); // remove carriage return
        json.addValue(st,"time");
        json.addValue("alarmstate","event");
        json.addValue((char*)&status,"status");
        json.addValue((char*)&from,"from");
    }
}


