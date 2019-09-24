#ifndef GOALY_H
#define GOALY_H

#include "../common/Vector2D.h"
#include "PlayerBase.h"
#include "StateMachine.h"

class PlayerBase;

class GoalKeeper : public PlayerBase
{
private:
    std::string mCurStateName;
   //an instance of the state machine class
  StateMachine<GoalKeeper>*  m_pStateMachine;
  
  //this vector is updated to point towards the ball and is used when
  //rendering the goalkeeper (instead of the underlaying vehicle's heading)
  //to ensure he always appears to be watching the ball
  Vector2D   m_vLookAt;

public:
  
   GoalKeeper(SoccerTeam* home_team, int home_region, State<GoalKeeper>* start_state,
              Vector2D   heading, Vector2D velocity,
              float mass, float  max_force, float max_speed, float max_turn_rate,
              float scale, std::string stateName);

   ~GoalKeeper(){delete m_pStateMachine;}

   //these must be implemented
   void        Update();
   bool        HandleMessage(const Telegram& msg);
   std::string GetCurStateName() { return mCurStateName;}
   void RecordCurStateName( std::string name){ mCurStateName = name; }
   //returns true if the ball comes close enough for the keeper to 
   //consider intercepting
   bool        BallWithinRangeForIntercept()const;

   //returns true if the keeper has ventured too far away from the goalmouth
   bool        TooFarFromGoalMouth()const;

   Vector2D    GetRearInterposeTarget()const;

   StateMachine<GoalKeeper>* GetFSM()const{return m_pStateMachine;}

   Vector2D    LookAt()const{return m_vLookAt;}
};



#endif