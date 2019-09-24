
#ifndef FIELDPLAYER_H
#define FIELDPLAYER_H

#include <vector>
#include <string>
#include <algorithm>
#include "FieldPlayerStates.h"
#include "../common/Vector2D.h"
#include "StateMachine.h"
#include "PlayerBase.h"
#include "StateMachine.h"
#include "../common/Regulator.h"

class CSteeringBehavior;
class SoccerTeam;
class SoccerPitch;
class Goal;
struct Telegram;


class FieldPlayer : public PlayerBase
{
private:
  std::string mCurStateName;
   //an instance of the state machine class
  StateMachine<FieldPlayer>*  m_pStateMachine;
  //limits the number of kicks a player may take per second
  Regulator*                  m_pKickLimiter;
  Vector2D      m_vLookAt;
public:

  FieldPlayer(SoccerTeam*    home_team, int  home_region, State<FieldPlayer>* start_state,
             Vector2D  heading, Vector2D    velocity,
             float   mass, float   max_force,
             float   max_speed, float  max_turn_rate,
             float   scale, player_role role, std::string stateName);
  
  ~FieldPlayer();

  //call this to update the player's position and orientation
  void        Update();
  bool        HandleMessage(const Telegram& msg);
  std::string GetCurStateName() { return mCurStateName;}
  void RecordCurStateName( std::string name){ mCurStateName = name; }
  StateMachine<FieldPlayer>* GetFSM()const{return m_pStateMachine;}
  bool        isReadyForNextKick()const{return m_pKickLimiter->isReady();}
  Vector2D LookAt() { return m_vLookAt; }
         
};




#endif