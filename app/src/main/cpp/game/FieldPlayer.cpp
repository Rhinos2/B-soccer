#include "FieldPlayer.h"
#include "PlayerBase.h"
#include "SteeringBehaviors.h"
#include "../common/Transformations.h"
#include "../common/geometry.h"

#include "../common/C2DMatrix.h"
#include "Goal.h"
#include "Region.h"
#include "../common/EntityFunctionTemplates.h"
#include "SoccerTeam.h"
#include "../common/Regulator.h"



#include <limits>

using std::vector;

//------------------------------- dtor ---------------------------------------
//----------------------------------------------------------------------------
FieldPlayer::~FieldPlayer()
{
  delete m_pKickLimiter;
  delete m_pStateMachine;
}

//----------------------------- ctor -------------------------------------
//------------------------------------------------------------------------
FieldPlayer::FieldPlayer(SoccerTeam* home_team,
                      int   home_region,
                      State<FieldPlayer>* start_state, Vector2D  heading, Vector2D velocity,
                      float    mass, float    max_force, float    max_speed,
                      float    max_turn_rate, float    scale, player_role role, std::string stateName):
        PlayerBase(home_team, home_region, heading, velocity, mass, max_force, max_speed,
                                                    max_turn_rate, scale, role)
{
  //set up the state machine
  m_pStateMachine =  new StateMachine<FieldPlayer>(this);

  if (start_state)
  {
    mCurStateName = stateName;
    m_pStateMachine->SetCurrentState(start_state);
    m_pStateMachine->SetPreviousState(start_state);
    m_pStateMachine->SetGlobalState(GlobalPlayerState::Instance());

    m_pStateMachine->CurrentState()->Enter(this);
  }    

  m_pSteering->SeparationOn();

  //set up the kick regulator
    m_pKickLimiter = new Regulator(2);//PlayerKickFrequency
}

//------------------------------ Update ----------------------------------
//
//  
//------------------------------------------------------------------------
void FieldPlayer::Update()
{ 
  //run the logic for the current state
  m_pStateMachine->Update();
  //calculate the combined steering force
    Vector2D SteeringForce = m_pSteering->Calculate();
  //if no steering force is produced decelerate the player by applying a
  //braking force
  if (m_pSteering->Force().isZero())
  {
    const float BrakingRate = 0.8;
    m_vVelocity = m_vVelocity * BrakingRate;                                     
  }

  //the steering force's side component is a force that rotates the 
  //player about its axis. We must limit the rotation so that a player
  //can only turn by PlayerMaxTurnRate rads per update.
  float TurningForce =   m_pSteering->SideComponent();
  //Clamp(TurningForce, - 0.8f, /*PlayerMaxTurnRate */0.8f); //PlayerMaxTurnRate
  //rotate the heading vector
  Vec2DRotateAroundOrigin(m_vHeading, TurningForce);

  //make sure the velocity vector points in the same direction as
  //the heading vector
  m_vVelocity = m_vHeading * m_vVelocity.Length();
  //and recreate m_vSide
  m_vSide = m_vHeading.Perp();

  //now to calculate the acceleration due to the force exerted by
  //the forward component of the steering force in the direction
  //of the player's heading
  Vector2D accel = m_vHeading * m_pSteering->ForwardComponent() / (m_dMass * 160);
  //for a quicker start
  if(Pitch()->GameOn() && !m_pSteering->Force().isZero() && Vec2DLengthSq(m_vVelocity) < 0.0005f)
      accel *= 6;
     m_vVelocity += accel;
  //make sure player does not exceed maximum velocity
  m_vVelocity.Truncate(0.05f);
  //update the position
  m_vPosition += m_vVelocity;
  m_vLookAt = Vec2DNormalize(Ball()->OldPos() - Pos());
}

//-------------------- HandleMessage -------------------------------------
//
//  routes any messages appropriately
//------------------------------------------------------------------------
bool FieldPlayer::HandleMessage(const Telegram& msg)
{
  return m_pStateMachine->HandleMessage(msg);
}




