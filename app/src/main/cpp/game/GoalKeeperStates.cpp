#include "GoalKeeperStates.h"

//#include "SoccerPitch.h"
#include "PlayerBase.h"
#include "Goalkeeper.h"
#include "SteeringBehaviors.h"
#include "SoccerTeam.h"
#include "Goal.h"
#include "../common/geometry.h"
#include "FieldPlayer.h"
//#include "ParamLoader.h"
#include "../common/Telegram.h"
#include "../common/MessageDispatcher.h"
#include "SoccerMessages.h"

//--------------------------- GlobalKeeperState -------------------------------
//-----------------------------------------------------------------------------

GlobalKeeperState* GlobalKeeperState::Instance()
{
  static GlobalKeeperState instance;
  return &instance;
}


bool GlobalKeeperState::OnMessage(GoalKeeper* keeper, const Telegram& telegram)
{
  switch(telegram.Msg)
  {
    case Msg_GoHome:
    {
      keeper->SetDefaultHomeRegion();
      keeper->GetFSM()->ChangeState(ReturnHome::Instance());
    }

    break;

    case Msg_ReceiveBall:
      {
        keeper->GetFSM()->ChangeState(InterceptBall::Instance());
      }

      break;

  }//end switch

  return false;
}


//--------------------------- TendGoal -----------------------------------
//
//  This is the main state for the goalkeeper. When in this state he will
//  move left to right across the goalmouth using the 'interpose' steering
//  behavior to put himself between the ball and the back of the net.
//
//  If the ball comes within the 'goalkeeper range' he moves out of the
//  goalmouth to attempt to intercept it. (see next state)
//------------------------------------------------------------------------

TendGoal* TendGoal::Instance()
{
  static TendGoal instance;
  return &instance;
}


void TendGoal::Enter(GoalKeeper* keeper)
{
  keeper->RecordCurStateName(std::string("T"));
  //turn interpose on
  keeper->Steering()->InterposeOn(0.5f);//GoalKeeperTendingDistance

  //interpose will position the agent between the ball position and a target
  //position situated along the goal mouth. This call sets the target
  keeper->Steering()->SetTarget(keeper->GetRearInterposeTarget());
}

void TendGoal::Execute(GoalKeeper* keeper)
{
    Vector2D BallVelocity(keeper->Pitch()->GetBallPtr()->Velocity());
  //---------------------------------if the ball comes in keeper's trap range
  if (keeper->BallWithinKeeperRange()) {
    // the ball is moving too fast for the keeper to try to catch it - deflect
    if(BallVelocity.LengthSq() > 0.023f) {
        float randCos = RandFloat();
        float randSin = RandFloat();
        Vector2D randVec((keeper->LookAt().x * randCos - keeper->LookAt().y * randSin),
                         (keeper->LookAt().x * randSin + keeper->LookAt().y * randCos));
        BallVelocity.Reflect(Vec2DNormalize(randVec));
        keeper->Pitch()->GetBallPtr()->SetVelocity(BallVelocity);
    }
    else { //ok to trap it and put it back in play
      keeper->Ball()->Trap();
      keeper->Pitch()->SetGoalKeeperHasBall(true);
      keeper->GetFSM()->ChangeState(PutBallBackInPlay::Instance());
    }
    return;
  }
  //--if ball's within a predefined distance, the keeper may want to moves out to try to intercept it
  else if(keeper->BallWithinRangeForIntercept() && keeper->isClosestPlayerOnPitchToBall()){
      keeper->GetFSM()->ChangeState(InterceptBall::Instance());
      return;
  }
}
void TendGoal::Exit(GoalKeeper* keeper)
{
  keeper->Steering()->InterposeOff();
}
//------------------------- ReturnHome: ----------------------------------
//
//  In this state the goalkeeper simply returns back to the center of
//  the goal region before changing state back to TendGoal
//------------------------------------------------------------------------

ReturnHome* ReturnHome::Instance()
{
  static ReturnHome instance;
  return &instance;
}
void ReturnHome::Enter(GoalKeeper* keeper)
{
  keeper->RecordCurStateName(std::string("H"));
  keeper->Steering()->ArriveOn();
}
void ReturnHome::Execute(GoalKeeper* keeper)
{
  keeper->Steering()->SetTarget(keeper->HomeRegion()->Center());

  //if close enough to home or the opponents get control over the ball,
  //change state to tend goal
  if (keeper->InHomeRegion())
  {
    keeper->GetFSM()->ChangeState(TendGoal::Instance());
  }
}
void ReturnHome::Exit(GoalKeeper* keeper)
{
  keeper->Steering()->ArriveOff();
}



//----------------- InterceptBall ----------------------------------------
//
//  In this state the GP will attempt to intercept the ball using the
//  pursuit steering behavior, but he only does so so long as he remains
//  within his home region.
//------------------------------------------------------------------------

InterceptBall* InterceptBall::Instance()
{
  static InterceptBall instance;
  return &instance;
}
void InterceptBall::Enter(GoalKeeper* keeper)
{
  keeper->RecordCurStateName(std::string("I"));
 // keeper->Steering()->SeekOn();
    keeper->Steering()->PursuitOn();
}

void InterceptBall::Execute(GoalKeeper* keeper)
{ 
  //if the goalkeeper moves to far away from the goal he should return to his
  //home region UNLESS he is the closest player to the ball, in which case,
  //he should keep trying to intercept it.
  if (keeper->TooFarFromGoalMouth() && !keeper->isClosestPlayerOnPitchToBall())
  {
    keeper->GetFSM()->ChangeState(ReturnHome::Instance());
    return;
  }
  //if the ball becomes in range of the goalkeeper's hands he traps the 
  //ball and puts it back in play
  if (keeper->BallWithinKeeperRange())
  {
    keeper->Ball()->Trap();
    keeper->SetVelocity(Vector2D(0.0f, 0.0f));
    keeper->Pitch()->SetGoalKeeperHasBall(true);
    keeper->GetFSM()->ChangeState(PutBallBackInPlay::Instance());

    return;
  }
}

void InterceptBall::Exit(GoalKeeper* keeper)
{
  keeper->Steering()->PursuitOff();
}



//--------------------------- PutBallBackInPlay --------------------------
//
//------------------------------------------------------------------------
int PutBallBackInPlay::count = 0;
PutBallBackInPlay* PutBallBackInPlay::Instance()
{
  static PutBallBackInPlay instance;
  return &instance;
}

void PutBallBackInPlay::Enter(GoalKeeper* keeper)
{
  keeper->RecordCurStateName(std::string("P"));
  //let the team know that the keeper is in control
  keeper->Team()->SetControllingPlayer(keeper);

  //send all the players home
  keeper->Team()->Opponents()->ReturnAllFieldPlayersToHome();
 // keeper->Team()->ReturnAllFieldPlayersToHome();//
  count = 0;
}


void PutBallBackInPlay::Execute(GoalKeeper* keeper)
{
  PlayerBase*  receiver = NULL;
  Vector2D     BallTarget;
    count++;
    if(count < 45){ //simulate a pause while the keeper decides what to do next
        return;
    }
  //test if there are players further forward on the field; we might be able to pass to. If so, make a pass.
  if (keeper->Team()->FindPass(keeper, receiver, BallTarget,  0.4f, /*MaxPassingForce*/ 0.5f/*MinPassingDist*/))
  {
    //make the pass   
    keeper->Ball()->Kick( BallTarget - keeper->Ball()->Pos(), 0.4f);//MaxPassingForce
    //goalkeeper no longer has ball 
    keeper->Pitch()->SetGoalKeeperHasBall(false);
    //let the receiving player know the ball's comin' at him
    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, keeper->ID(), receiver->ID(), Msg_ReceiveBall,
                            BallTarget, &BallTarget);
    //go back to tending the goal
    keeper->GetFSM()->ChangeState(TendGoal::Instance());
    return;
  }
  else  //a random kick forward
    {
      Vector2D Center (0.0f, 0.0f);

      BallTarget = AddNoiseToKick(keeper->Ball()->Pos(), Center);
      Vector2D KickDirection = Center - keeper->Ball()->Pos();
      keeper->Ball()->Kick(KickDirection, 0.4f); //max kicking force
      keeper->Pitch()->SetGoalKeeperHasBall(false);
      keeper->GetFSM()->ChangeState(TendGoal::Instance());
    }

  return;
}

