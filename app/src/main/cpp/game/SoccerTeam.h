#ifndef SOCCERTEAM_H
#define SOCCERTEAM_H


#include <vector>
#include "../GameApp.h"

#include "Region.h"
#include "SupportSpotCalculator.h"
#include "StateMachine.h"

class GameApp;
class Goal;
class PlayerBase;
class FieldPlayer;
class SoccerPitch;
class GoalKeeper;
class SupportSpotCalculator;

                
class SoccerTeam 
{
public:

  enum team_color {blue, red};

private:

   //an instance of the state machine class
  StateMachine<SoccerTeam>*  m_pStateMachine;

  //the team must know its own color!
  team_color                m_Color;

  //pointers to the team members
  std::vector<PlayerBase*>  m_Players;

  //a pointer to the soccer pitch
  GameApp*              m_pPitch;

  //pointers to the goals
  Goal*                     m_pOpponentsGoal;
  Goal*                     m_pHomeGoal;
  
  //a pointer to the opposing team
  SoccerTeam*               m_pOpponents;
   
  //pointers to 'key' players
  PlayerBase*               m_pControllingPlayer;
  PlayerBase*               m_pSupportingPlayer;
  PlayerBase*               m_pReceivingPlayer;
  PlayerBase*               m_pPlayerClosestToBall;

  //the squared distance the closest player is from the ball
  float                     m_dDistSqToBallOfClosestPlayer;
  //players use this to determine strategic positions on the playing field
  SupportSpotCalculator*    m_pSupportSpotCalc;

  //creates all the players for this team
  void CreatePlayers();
  //called each frame. Sets m_pClosestPlayerToBall to point to the player
  //closest to the ball. 
  void CalculateClosestPlayerToBall();

public:

    SupportSpotCalculator* GetSupportCalcObject() { return m_pSupportSpotCalc;}

  SoccerTeam(Goal* home_goal, Goal* opponents_goal, GameApp* pitch, team_color   color);

  ~SoccerTeam();
  void        Update();
  //calling this changes the state of all field players to that of 
  //ReturnToHomeRegion. Mainly used when a goal keeper has
  //possession
  void        ReturnAllFieldPlayersToHome()const;

  //returns true if player has a clean shot at the goal and sets ShotTarget
  //to a normalized vector pointing in the direction the shot should be
  //made. Else returns false and sets heading to a zero vector
  bool        CanShoot(Vector2D  BallPos, float  power,  Vector2D& ShotTarget )const;

  bool        FindPass(const PlayerBase*const passer, PlayerBase*& receiver, Vector2D& PassTarget,
                      float power, float  MinPassingDistance)const;

  //Three potential passes are calculated. One directly toward the receiver's
  //current position and two that are the tangents from the ball position
  //to the circle of radius 'range' from the receiver.
  //These passes are then tested to see if they can be intercepted by an
  //opponent and to make sure they terminate within the playing area. If
  //all the passes are invalidated the function returns false. Otherwise
  //the function returns the pass that takes the ball closest to the 
  //opponent's goal area.
  bool        GetBestPassToReceiver( const PlayerBase*  passer,
                                     const PlayerBase*  receiver,
                                    Vector2D& PassTarget,
                                     const float power)const;

  //test if a pass from positions 'from' to 'target' kicked with force 
  //'PassingForce'can be intercepted by an opposing player
  bool        isPassSafeFromOpponent(Vector2D    from,
                                     Vector2D    target,
                                     const PlayerBase* const receiver,
                                     const PlayerBase* const opp,
                                     float       PassingForce)const;

  //tests a pass from position 'from' to position 'target' against each member
  //of the opposing team. Returns true if the pass can be made without
  //getting intercepted
  bool        isPassSafeFromAllOpponents(Vector2D from,
                                         Vector2D target,
                                         const PlayerBase* const receiver,
                                         float     PassingForce)const;

  //returns true if there is an opponent within radius of position
  bool        isOpponentWithinRadius(Vector2D pos, float rad);

  //this tests to see if a pass is possible between the requester and
  //the controlling player. If it is possible a message is sent to the
  //controlling player to pass the ball asap.
  void        RequestPass(FieldPlayer* requester)const;

  //calculates the best supporting position and finds the most appropriate
  //attacker to travel to the spot
  PlayerBase* DetermineBestSupportingAttacker();
  

  const std::vector<PlayerBase*>& Members()const{return m_Players;}  

  StateMachine<SoccerTeam>* GetFSM()const{return m_pStateMachine;}
  
  Goal*const           HomeGoal()const{return m_pHomeGoal;}
  Goal*const           OpponentsGoal()const{return m_pOpponentsGoal;}

  GameApp*const    Pitch()const{return m_pPitch;}

  SoccerTeam*const     Opponents()const{return m_pOpponents;}
  void                 SetOpponents(SoccerTeam* opps){m_pOpponents = opps;}

  team_color           Color()const{return m_Color;}

  void                 SetPlayerClosestToBall(PlayerBase* plyr){m_pPlayerClosestToBall=plyr;}
  PlayerBase*          PlayerClosestToBall()const{return m_pPlayerClosestToBall;}
  
  float               ClosestDistToBallSq()const{return m_dDistSqToBallOfClosestPlayer;}

  Vector2D             GetSupportSpot()const{return m_pSupportSpotCalc->GetBestSupportingSpot();}

  PlayerBase*          SupportingPlayer()const{return m_pSupportingPlayer;}
  void                 SetSupportingPlayer(PlayerBase* plyr){m_pSupportingPlayer = plyr;}

  PlayerBase*          Receiver()const{return m_pReceivingPlayer;}
  void                 SetReceiver(PlayerBase* plyr){m_pReceivingPlayer = plyr;}

  PlayerBase*          ControllingPlayer()const{return m_pControllingPlayer;}
  void                 SetControllingPlayer(PlayerBase* plyr)
  {
    m_pControllingPlayer = plyr;
    Opponents()->LostControl();
  }


  bool  InControl()const{
    if(m_pControllingPlayer)
    return true;
  else
    return false;}
  void  LostControl(){m_pControllingPlayer = NULL;}

  PlayerBase*  GetPlayerFromID(int id)const;
  void SetPlayerHomeRegion(int plyr, int region)const;
  void DetermineBestSupportingPosition()const{m_pSupportSpotCalc->DetermineBestSupportingPosition();}
  void UpdateTargetsOfWaitingPlayers()const;
  //returns false if any of the team are not located within their home region
  bool AllPlayersAtHome()const;
  std::string Name()const{if (m_Color == blue) return "Blue"; return "Red";}

};

#endif