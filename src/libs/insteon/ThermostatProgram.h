#ifndef THERMOSTATPROGRAM_H
#define THERMOSTATPROGRAM_H
#include <vector>
#include <map>
#include <pthread.h>
#define HAVE_CXX_STDHEADERS
#include <db_cxx.h>
#include "../json/JSON.h"

struct ProgramPoint
{
    unsigned char mode;
    unsigned char temp;
};

class ThermostatProgram{
private:
  std::map<int,ProgramPoint> mEventList;
  DB *mpDB;
  pthread_mutex_t mListLock;
    bool mActive;
public:
  ThermostatProgram();
  ~ThermostatProgram();

    void setPoint(unsigned char absoluteTime, unsigned char temp, unsigned char mode);
    void removePoint(unsigned char absoluteTime);
    ProgramPoint getPoint(unsigned char absoluteTime);
    void setActive(bool active);
    bool isActive();
};

#endif

