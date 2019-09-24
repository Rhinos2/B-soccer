
#include <stdlib.h>
#include "PlayerBase.h"
#include "SteeringBehaviors.h"
#include "../common/Transformations.h"
#include "../common/geometry.h"
#include "../common/C2DMatrix.h"
#include "Region.h"
#include "../common/MessageDispatcher.h"
#include "SoccerMessages.h"
#include "SoccerTeam.h"
#include "Goal.h"
#include "SoccerBall.h"
//#include "SoccerPitch.h"



using std::vector;


//----------------------------- dtor -------------------------------------
//------------------------------------------------------------------------
PlayerBase::~PlayerBase()
{
  delete m_pSteering;
 // fclose(stream);
}

//----------------------------- ctor -------------------------------------
//------------------------------------------------------------------------
PlayerBase::PlayerBase(SoccerTeam* home_team,
                       int   home_region,
                       Vector2D  heading,
                       Vector2D velocity,
                       float    mass,
                       float    max_force,
                       float    max_speed,
                       float    max_turn_rate,
                       float    scale,
                       player_role role):    

    MovingEntity(home_team->Pitch()->GetRegionFromIndex(home_region)->Center(),
                 scale*1.0f,//not used
                 velocity,
                 max_speed,
                 heading,
                 mass,
                 Vector2D(scale,scale),
                 max_turn_rate,
                 max_force),
   m_pTeam(home_team),
   m_dDistSqToBall(MaxFloat),
   m_iHomeRegion(home_region),
   m_iDefaultRegion(home_region),
   m_PlayerRole(role)
{

  //set up the steering behavior class
  m_pSteering = new SteeringBehaviors(this, m_pTeam->Pitch(), Ball());
  
  //a player's start target is its start position (because it's just waiting)
  m_pSteering->SetTarget(home_team->Pitch()->GetRegionFromIndex(home_region)->Center());
}


//----------------------------- TrackBall --------------------------------
//
//  sets the player's heading to point at the ball
//------------------------------------------------------------------------
void PlayerBase::TrackBall()
{
  RotateHeadingToFacePosition(Ball()->Pos());  
}

//----------------------------- TrackTarget --------------------------------
//
//  sets the player's heading to point at the current target
//------------------------------------------------------------------------
void PlayerBase::TrackTarget()
{
  SetHeading(Vec2DNormalize(Steering()->Target() - Pos()));
}


//------------------------------------------------------------------------
//
//binary predicates for std::sort (see CanPassForward/Backward)
//------------------------------------------------------------------------
bool  SortByDistanceToOpponentsGoal(const PlayerBase*const p1,
                                    const PlayerBase*const p2)
{
  return (p1->DistToOppGoal() < p2->DistToOppGoal());
}

bool  SortByReversedDistanceToOpponentsGoal(const PlayerBase*const p1,
                                            const PlayerBase*const p2)
{
  return (p1->DistToOppGoal() > p2->DistToOppGoal());
}


//------------------------- WithinFieldOfView ---------------------------
//
//  returns true if subject is within field of view of this player
//-----------------------------------------------------------------------
bool PlayerBase::PositionInFrontOfPlayer(Vector2D position)const
{
  Vector2D ToSubject = position - Pos();

  if (ToSubject.Dot(Heading()) > 0) 
    
    return true;

  else

    return false;
}

//------------------------- IsThreatened ---------------------------------
//
//  returns true if there is an opponent within this player's 
//  comfort zone
//------------------------------------------------------------------------
bool PlayerBase::isThreatened()const
{
  //check against all opponents to make sure non are within this
  //player's comfort zone
  std::vector<PlayerBase*>::const_iterator curOpp;  
  curOpp = Team()->Opponents()->Members().begin();
 
  for (curOpp; curOpp != Team()->Opponents()->Members().end(); ++curOpp)
  {
    //calculate distance to the player. if dist is less than our
    //comfort zone, and the opponent is infront of the player, return true
    if (PositionInFrontOfPlayer((*curOpp)->Pos()) && (Vec2DDistanceSq(Pos(), (*curOpp)->Pos()) < 0.8f*0.8f))//PlayerComfortZone
      return true;
  }// next opp
  return false;
}

//----------------------------- FindSupport -----------------------------------
//
//  determines the player who is closest to the SupportSpot and messages him
//  to tell him to change state to SupportAttacker
//-----------------------------------------------------------------------------
void PlayerBase::FindSupport()const
{    
  //if there is no support we need to find a suitable player.
  if (Team()->SupportingPlayer() == NULL)
  {
    PlayerBase* BestSupportPly = Team()->DetermineBestSupportingAttacker();
    Team()->SetSupportingPlayer(BestSupportPly);
  }
    Vector2D	place;
    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, ID(), Team()->SupportingPlayer()->ID(),
                            Msg_SupportAttacker, place, NULL);

    
  PlayerBase* BestSupportPly = Team()->DetermineBestSupportingAttacker();
  if (BestSupportPly && (BestSupportPly != Team()->SupportingPlayer()))
  {
    if (Team()->SupportingPlayer())
    {
      Vector2D place_home;
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, ID(), Team()->SupportingPlayer()->ID(),
                              Msg_GoHome, place_home, NULL);
    }

    Team()->SetSupportingPlayer(BestSupportPly);
    Vector2D place_support;

    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, ID(), Team()->SupportingPlayer()->ID(),
                            Msg_SupportAttacker, place_support, NULL);
  }
}


  //calculate distance to opponent's goal. Used frequently by the passing//methods
float PlayerBase::DistToOppGoal()const
{
  return fabs(Pos().y - Team()->OpponentsGoal()->Center().y);
}

float PlayerBase::DistToHomeGoal()const
{
  return fabs(Pos().y - Team()->HomeGoal()->Center().y);
}

bool PlayerBase::isControllingPlayer()const
{return Team()->ControllingPlayer()==this;}

bool PlayerBase::BallWithinKeeperRange()const
{
  return (Vec2DDistanceSq(Pos(), Ball()->Pos()) < 0.3f * 0.3f);//KeeperRange
}

bool PlayerBase::BallWithinReceivingRange()const
{
  return (Vec2DDistanceSq(Pos(), Ball()->Pos()) < 0.2f * 0.2f);
}

bool PlayerBase::BallWithinKickingRange()
{
  return (Vec2DDistanceSq(Ball()->Pos(), Pos()) - 0.2f < 0.0f);
}


bool PlayerBase::InHomeRegion()const
{
  if (m_PlayerRole == goal_keeper)
  {
    return Pitch()->GetRegionFromIndex(m_iHomeRegion)->Inside(Pos(), Region::normal);
  }
  else
  {
    return Pitch()->GetRegionFromIndex(m_iHomeRegion)->Inside(Pos(), Region::halfsize);
  }
}

bool PlayerBase::AtTarget()const
{
  return (Vec2DDistanceSq(Pos(), Steering()->Target()) < 0.3f * 0.3f);
}

bool PlayerBase::isClosestTeamMemberToBall()const
{
  return Team()->PlayerClosestToBall() == this;
}

bool PlayerBase::isClosestPlayerOnPitchToBall()const
{
  return isClosestTeamMemberToBall() && 
         (DistSqToBall() < Team()->Opponents()->ClosestDistToBallSq());
}

bool PlayerBase::InHotRegion()const
{
  return fabs(Pos().y - Team()->OpponentsGoal()->Center().y ) <
         Pitch()->PlayingArea()->Length()/3.0;
}

bool PlayerBase::isAheadOfAttacker()const
{
  return fabs(Pos().y - Team()->OpponentsGoal()->Center().y) <
         fabs(Team()->ControllingPlayer()->Pos().y - Team()->OpponentsGoal()->Center().y);
}

SoccerBall* const PlayerBase::Ball()const
{
  return Team()->Pitch()->Ball();
}

GameApp* const PlayerBase::Pitch()const
{
  return Team()->Pitch();
}

const Region* const PlayerBase::HomeRegion()const
{
  return Pitch()->GetRegionFromIndex(m_iHomeRegion);
}


