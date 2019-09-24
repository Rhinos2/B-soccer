#include "SupportSpotCalculator.h"
#include "PlayerBase.h"
#include "Goal.h"
#include "SoccerBall.h"
#include "ConstantsToo.h"
#include "../common/Regulator.h"
#include "SoccerTeam.h"

//#include "SoccerPitch.h"



//------------------------------- dtor ----------------------------------------
//-----------------------------------------------------------------------------
SupportSpotCalculator::~SupportSpotCalculator()
{
  delete m_pRegulator;
}


//------------------------------- ctor ----------------------------------------
//-----------------------------------------------------------------------------
SupportSpotCalculator::SupportSpotCalculator(int numX, int numZ, SoccerTeam*   team):
        m_pBestSupportingSpot(NULL),
        m_pTeam(team)
{
  const Region* PlayingField = team->Pitch()->PlayingArea();

  float HeightOfSSRegion = PlayingField->Height() * 0.8f;
  float WidthOfSSRegion  = PlayingField->Width() * 0.9f;
  float SliceX = WidthOfSSRegion / numX ;
  float SliceZ = HeightOfSSRegion / numZ;

  float left  = PlayingField->Left() - (PlayingField->Width()-WidthOfSSRegion)/2.0f - SliceX/2.0f;
  //float right = PlayingField->Right() + (PlayingField->Width()-WidthOfSSRegion)/2.0f + SliceX/2.0f;
  float top   = PlayingField->Top() - (PlayingField->Height()-HeightOfSSRegion)/2.0f - SliceZ/2.0f;
  float bottom   = PlayingField->Bottom() + (PlayingField->Height()-HeightOfSSRegion)/2.0f + SliceZ/2.0f;


  for (int z=0; z<(numZ/2)-1; z++)
  {
    for (int x=0; x<numX; x++)
    {
      if (m_pTeam->Color() == SoccerTeam::red)
          m_Spots.push_back(SupportSpot(Vector2D(left-x*SliceX, bottom+z*SliceZ), 0.0f));
      else
          m_Spots.push_back(SupportSpot(Vector2D(left-x*SliceX, top-z*SliceZ), 0.0f));
    }
  }
  
  //create the regulator
  m_pRegulator = new Regulator(1.0f); //SupportSpotUpdateFreq is once per sec
}


//--------------------------- DetermineBestSupportingPosition -----------------
//
//  see header or book for description
//-----------------------------------------------------------------------------
Vector2D SupportSpotCalculator::DetermineBestSupportingPosition()
{

  if (m_pBestSupportingSpot)
  {
    return m_pBestSupportingSpot->m_vPos;
  }

  //reset the best supporting spot
  m_pBestSupportingSpot = NULL;
 
  float BestScoreSoFar = 0.0f;

  std::vector<SupportSpot>::iterator curSpot;

  for (curSpot = m_Spots.begin(); curSpot != m_Spots.end(); ++curSpot)
  {
    curSpot->m_dScore = 1.0;
    //Test 1. is it possible to make a safe pass from the ball's position to this position?

    if(m_pTeam->isPassSafeFromAllOpponents(m_pTeam->ControllingPlayer()->Pos(), curSpot->m_vPos,
                                           NULL, 0.3f))//MaxPassingForce
    {
      curSpot->m_dScore += 10.0f;//Spot_PassSafeScore
    }

    //Test 2. Determine if a goal can be scored from this position.
    Vector2D target;
    if( m_pTeam->CanShoot(curSpot->m_vPos, 0.5f, target))//MaxShootingForce
    {
      curSpot->m_dScore += 2.0f;
    }
    
    //check to see if this spot has the highest score so far
    if (curSpot->m_dScore > BestScoreSoFar)
    {
      BestScoreSoFar = curSpot->m_dScore;
      m_pBestSupportingSpot = &(*curSpot);
    }    
    
  }

  return m_pBestSupportingSpot->m_vPos;
}





//------------------------------- GetBestSupportingSpot -----------------------
//-----------------------------------------------------------------------------
Vector2D SupportSpotCalculator::GetBestSupportingSpot()
{
  if (m_pBestSupportingSpot)
  {
    return m_pBestSupportingSpot->m_vPos;
  }
    
  else
  { 
    return DetermineBestSupportingPosition();
  }
}

