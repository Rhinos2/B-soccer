
#ifndef PLAYERBASE_H
#define PLAYERBASE_H

#include <vector>
#include <string>
#include "../GameApp.h"

#include "../common/autolist.h"
#include "../common/Vector2D.h"
#include "../common/MovingEntity.h"

class GameApp;
class SoccerTeam;
class SoccerPitch;
class SoccerBall;
class SteeringBehaviors;
class Region;



class PlayerBase : public MovingEntity, public AutoList<PlayerBase>
{

public:
  
  enum player_role{goal_keeper, attacker, defender};

protected:

  //this player's role in the team
  player_role             m_PlayerRole;

  //a pointer to this player's team
  SoccerTeam*             m_pTeam;
 
  //the steering behaviors
  SteeringBehaviors*      m_pSteering;

  //the region that this player is assigned to.
  int                     m_iHomeRegion;

  //the region this player moves to before kickoff
  int                     m_iDefaultRegion;

  //the distance to the ball (in squared-space). This value is queried 
  //a lot so it's calculated once each time-step and stored here.
  float                   m_dDistSqToBall;

  
  //the vertex buffer
  std::vector<Vector2D>   m_vecPlayerVB;
  //the buffer for the transformed vertices
  std::vector<Vector2D>   m_vecPlayerVBTrans;

public:

  PlayerBase(SoccerTeam*    home_team,
             int            home_region,
             Vector2D       heading,
             Vector2D       velocity,
             float          mass,
             float          max_force,
             float          max_speed,
             float          max_turn_rate,
             float          scale,
             player_role    role);

  virtual ~PlayerBase();


  //returns true if there is an opponent within this player's 
  //comfort zone
  bool        isThreatened()const;

  //rotates the player to face the ball or the player's current target
  void        TrackBall();
  void        TrackTarget();

  //this messages the player that is closest to the supporting spot to
  //change state to support the attacking player
  void        FindSupport()const;

  //returns true if the ball can be grabbed by the goalkeeper
  bool        BallWithinKeeperRange()const;

  //returns true if the ball is within kicking range
  bool        BallWithinKickingRange();

  //returns true if a ball comes within range of a receiver
  bool        BallWithinReceivingRange()const;

  //returns true if the player is located within the boundaries 
  //of his home region
  bool        InHomeRegion()const;

  //returns true if this player is ahead of the attacker
  bool        isAheadOfAttacker()const;
  
  //returns true if a player is located at the designated support spot
  bool        AtSupportSpot()const;

  //returns true if the player is located at his steering target
  bool        AtTarget()const;

  //returns true if the player is the closest player in his team to
  //the ball
  bool        isClosestTeamMemberToBall()const;

  //returns true if the point specified by 'position' is located in
  //front of the player
  bool        PositionInFrontOfPlayer(Vector2D position)const;

  //returns true if the player is the closest player on the pitch to the ball
  bool        isClosestPlayerOnPitchToBall()const;

  //returns true if this player is the controlling player
  bool        isControllingPlayer()const;

  //returns true if the player is located in the designated 'hot region' --
  //the area close to the opponent's goal
  bool        InHotRegion()const;

  player_role Role()const{return m_PlayerRole;}

  float       DistSqToBall()const{return m_dDistSqToBall;}
  void        SetDistSqToBall(float val){m_dDistSqToBall = val;}

  //calculate distance to opponent's/home goal. Used frequently by the passing
  //methods
  float       DistToOppGoal()const;
  float       DistToHomeGoal()const;

  void        SetDefaultHomeRegion(){m_iHomeRegion = m_iDefaultRegion;}
  int         GetHomeRegionID() { return m_iHomeRegion; }
  SoccerBall* const        Ball()const;
  GameApp* const       Pitch()const;
  SteeringBehaviors*const  Steering()const{return m_pSteering;}
  const Region* const      HomeRegion()const;
  void                     SetHomeRegion(int NewRegion){m_iHomeRegion = NewRegion;}
  SoccerTeam*const         Team()const{return m_pTeam;}
  FILE* stream;
};





#endif