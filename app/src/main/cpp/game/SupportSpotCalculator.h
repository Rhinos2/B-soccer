#ifndef SUPPORTSPOTCALCULATOR
#define SUPPORTSPOTCALCULATOR


#include <vector>

#include "Region.h"
#include "../common/Vector2D.h"



class PlayerBase;
class Goal;
class SoccerBall;
class SoccerTeam;
class Regulator;



class SupportSpotCalculator
{
public:
  
  //a data structure to hold the values and positions of each spot
  struct SupportSpot
  {
    Vector2D  m_vPos;
    float    m_dScore;

    SupportSpot(Vector2D pos, float value):m_vPos(pos),
                                            m_dScore(value)
    {}
  };

private:


  SoccerTeam*               m_pTeam;

  std::vector<SupportSpot>  m_Spots;

  //a pointer to the highest valued spot from the last update
  SupportSpot*              m_pBestSupportingSpot;

  //this will regulate how often the spots are calculated (default is
  //one update per second)
  Regulator*                m_pRegulator;

public:
  
  SupportSpotCalculator(int numX, int numY, SoccerTeam* team);

  ~SupportSpotCalculator();

  //this method iterates through each possible spot and calculates its
  //score.
  Vector2D  DetermineBestSupportingPosition();

  //returns the best supporting spot if there is one. If one hasn't been
  //calculated yet, this method calls DetermineBestSupportingPosition and
  //returns the result.
  Vector2D  GetBestSupportingSpot();
};


#endif