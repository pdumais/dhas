#ifndef SCHEDULE_H
#define SCHEDULE_H
#include "ScheduledEvent.h"
#include <vector>
#include <map>
#include <pthread.h>
#include "json/JSON.h"

class Schedule{
private:
  std::map<int,ScheduledEvent*> mEventList;
  DB *mpDB;
    pthread_mutex_t mListLock;
public:
  Schedule(Dumais::JSON::JSON& json);
  ~Schedule();

  void addEvent(ScheduledEvent* pEvent);
  void removeEvent(int eventID);
  
  std::vector<ScheduledEvent> getDueEvents(time_t t);
    
    std::vector<ScheduledEvent> getAllEvents();

};

#endif

