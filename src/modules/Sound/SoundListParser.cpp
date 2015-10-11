#include "DHASLogging.h"
#include "SoundListParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

/* Only support number sounds from -59 to 59 right now
 * Files needed for digits: 'minus', 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,30,40,50
 *
 *
 *
 *
 */
SoundListParser::SoundListParser(std::string url){
   
    size_t pos = 0;
    while(url!="")
    {
        pos = url.find(',');
        std::string token = url.substr(0,pos);
        if (pos!=std::string::npos) url = url.substr(pos+1); else url="";

        char *c=0;
        int num = strtol(token.c_str(),&c,0);
        if (c!=0 && c[0]==0)
        {
            if (num>-60 && num<60){
                if (num<0)
                {
                    mList.push_back("minus");
                    num = 0-num;  
                }

                if (num>=0 && num<20)
                {
                    std::stringstream ss;
                    ss << num;
                    mList.push_back(ss.str().c_str());
                }
                else if (num>=20 && num<70)
                {
                    int d = num/10;
                    num = num-(d*10);

                    char digits[3];
                    digits[0]=d+48;
                    digits[1]='0';
                    digits[2]=0;
                    mList.push_back((char*)&digits);
                    if (num!=0)
                    {
                        digits[0]=num+48;
                        digits[1]=0;
                        mList.push_back((char*)&digits);
                    }
                }
            }
            else 
            {
                mList.push_back("NaN");
            }
        } else {
            mList.push_back(token);
        }

    }

}

SoundListParser::~SoundListParser(){
}

std::vector<std::string>& SoundListParser::getSoundList()
{
    return mList;
}

