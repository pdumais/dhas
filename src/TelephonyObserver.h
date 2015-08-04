#pragma once

#include "Subscription.h"

class TelephonyObserver
{
public:
  virtual void onMWI(int num)=0;
  virtual void onConnectedUas(Call *pCall)=0;
  virtual void onAnswer(Call *pCall)=0;
  virtual void onDigit(Call *pCall, std::string digit)=0;
  virtual bool onNewCallUas(Call *pCall)=0;
  virtual bool onNewCallUac(Call *pCall)=0;
  virtual void onCallTerminated(Call *pCall)=0;

  virtual void onPresence(Subscription *pSub)=0;
  virtual void onNewDevicePresence(Subscription *pSub)=0;
};
