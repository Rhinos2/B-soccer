#include "Goalkeeper.h"

#include "SteeringBehaviors.h"
#include "SoccerTeam.h"
//#include "SoccerPitch.h"
#include "../common/Transformations.h"
#include "GoalKeeperStates.h"
#include "Goal.h"
#include "../common/EntityFunctionTemplates.h"
//#include "ParamLoader.h"



//----------------------------- ctor ------------------------------------
//-----------------------------------------------------------------------
GoalKeeper::GoalKeeper(SoccerTeam*  home_team, int home_region, State<GoalKeeper>* start_state,
                       Vector2D  heading, Vector2D velocity, float mass,
                       float  max_force, float max_speed, float  max_turn_rate,
                       float scale, std::string stateName):
        PlayerBase(home_team, home_region, heading, velocity, mass, max_force,
                   max_speed, max_turn_rate, scale, PlayerBase::goal_keeper)
                                         
                                        
{   
   //set up the state machine
  m_pStateMachine = new StateMachine<GoalKeeper>(this);
  mCurStateName = stateName;
  m_pStateMachine->SetCurrentState(start_state);
  m_pStateMachine->SetPreviousState(start_state);
  m_pStateMachine->SetGlobalState(GlobalKeeperState::Instance());

  m_pStateMachine->CurrentState()->Enter(this);        
}



//-------------------------- Update --------------------------------------

void GoalKeeper::Update()
{ 
  //run the logic for the current state
  m_pStateMachine->Update();

  //calculate the combined force from each steering behavior 
  Vector2D SteeringForce = m_pSteering->Calculate();

  //Acceleration = Force/Mass
  Vector2D Acceleration = SteeringForce / (m_dMass * 3);

  //update velocity
  m_vVelocity += Acceleration;

  //make sure player does not exceed maximum velocity
  m_vVelocity.Truncate(0.1f);  //(m_dMaxSpeed)

  //update the position
  m_vPosition += m_vVelocity;//
  // limit the goalie's movements
  if(m_vPosition.x > 1.4f)
      m_vPosition.x = 1.4f;
  else if(m_vPosition.x < -1.4f)
      m_vPosition.x = -1.4f;

  if(m_vPosition.y > 6.0f)
      m_vPosition.y = 6.0f;
  else if(m_vPosition.y < -6.0f)
      m_vPosition.y = -6.0f;
  //update the heading if the player has a non zero velocity
  if ( !m_vVelocity.isZero())
  {    
    m_vHeading = Vec2DNormalize(m_vVelocity);
    m_vSide = m_vHeading.Perp();
  }
   m_vLookAt = Vec2DNormalize(Ball()->OldPos() - Pos());

}


bool GoalKeeper::BallWithinRangeForIntercept()const
{
  return (Vec2DDistanceSq(Team()->HomeGoal()->Center(), Ball()->Pos()) <=
	  1.8f *1.8f);//GoalKeeperInterceptRangeSq
}

bool GoalKeeper::TooFarFromGoalMouth()const
{
  float xPosTarget = Team()->HomeGoal()->Center().x;
  float  yPosTarget = Team()->HomeGoal()->Center().y;;

  return (Vec2DDistanceSq(Pos(), Vector2D(xPosTarget, yPosTarget)) >
          1.5f *1.5f); //GoalKeeperInterceptRangeSq
}

Vector2D GoalKeeper::GetRearInterposeTarget()const
{
  float xPosTarget = Team()->HomeGoal()->Center().x;
 float  yPosTarget;
   if(Team()->Color() == Team()->red )
        yPosTarget = Team()->HomeGoal()->Center().y;
   else
        yPosTarget = Team()->HomeGoal()->Center().y;
  return Vector2D(xPosTarget, yPosTarget);
}

//-------------------- HandleMessage -------------------------------------
//
//  routes any messages appropriately
//------------------------------------------------------------------------
bool GoalKeeper::HandleMessage(const Telegram& msg)
{
  return m_pStateMachine->HandleMessage(msg);
}

