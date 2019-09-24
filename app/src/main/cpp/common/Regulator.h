#ifndef REGULATOR
#define REGULATOR

#include <sys/time.h>
#include "utils.h"


class Regulator
{
private:

  //the time period between updates 
  float m_dUpdatePeriod;

  //the next time the regulator allows code flow
  float m_dwNextUpdateTime;

    unsigned systemTime()
    {
      struct timeval tv;
      gettimeofday(&tv, 0);
      return (unsigned long)tv.tv_sec * 1000 + tv.tv_usec/1000;

    }
public:

  
  Regulator(float NumUpdatesPerSecondRqd)
  {
    m_dwNextUpdateTime = systemTime()+RandFloat()*1000;

    if (NumUpdatesPerSecondRqd > 0)
    {
      m_dUpdatePeriod = 1000.0f / NumUpdatesPerSecondRqd;
    }

    else if (isEqual(0.0, NumUpdatesPerSecondRqd))
    {
      m_dUpdatePeriod = 0.0;
    }

    else if (NumUpdatesPerSecondRqd < 0)
    {
      m_dUpdatePeriod = -1;
    }
  }


  //returns true if the current time exceeds m_dwNextUpdateTime
  bool isReady()
  {
    //if a regulator is instantiated with a zero freq then it goes into
    //stealth mode (doesn't regulate)
    if (isEqual(0.0, m_dUpdatePeriod)) return true;

    //if the regulator is instantiated with a negative freq then it will
    //never allow the code to flow
    if (m_dUpdatePeriod < 0) return false;

    unsigned CurrentTime = systemTime();

    //the number of milliseconds the update period can vary per required
    //update-step. This is here to make sure any multiple clients of this class
    //have their updates spread evenly
    static const float UpdatePeriodVariator = 10.0f;

    if (CurrentTime >= m_dwNextUpdateTime)
    {
      m_dwNextUpdateTime = (CurrentTime + m_dUpdatePeriod + RandInRange(-UpdatePeriodVariator, UpdatePeriodVariator));

      return true;
    }

    return false;
  }
};



#endif