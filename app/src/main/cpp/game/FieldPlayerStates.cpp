#include "FieldPlayerStates.h"

#include "FieldPlayer.h"
#include "SteeringBehaviors.h"
#include "SoccerTeam.h"
#include "Goal.h"
#include "../common/geometry.h"
#include "SoccerBall.h"
#include "../common/Telegram.h"
#include "../common/MessageDispatcher.h"
#include "SoccerMessages.h"

#include "../common/Regulator.h"
#include "Region.h"



//************************************************************************ Global state

GlobalPlayerState* GlobalPlayerState::Instance()
{
  static GlobalPlayerState instance;
  return &instance;
}


void GlobalPlayerState::Execute(FieldPlayer* player)                                     
{

}


bool GlobalPlayerState::OnMessage(FieldPlayer* player, const Telegram& telegram)
{
  switch(telegram.Msg)
  {
  case Msg_ReceiveBall:
    {
      //set the target
      player->Steering()->SetTarget(*(static_cast<Vector2D*>(telegram.ExtraInfo)));
      //change state 
      player->GetFSM()->ChangeState(ReceiveBall::Instance());
      return true;
    }

    break;

  case Msg_SupportAttacker:
    {
      //if already supporting just return
      if (player->GetFSM()->isInState(*SupportAttacker::Instance()))
      {
        return true;
      }
      
      //set the target to be the best supporting position
      player->Steering()->SetTarget(player->Team()->GetSupportSpot());
      //change the state
      player->GetFSM()->ChangeState(SupportAttacker::Instance());
      return true;
    }

    break;

 case Msg_Wait:
    {
      //change the state
      player->GetFSM()->ChangeState(Wait::Instance());
      return true;
    }

    break;

  case Msg_GoHome:
    {
      player->SetDefaultHomeRegion();
      player->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());
      return true;
    }

    break;

  case Msg_PassToMe:
    {
      //get the position of the player requesting the pass 
        FieldPlayer* receiver = static_cast<FieldPlayer*>(telegram.ExtraInfo);
        Vector2D position = telegram.dummy;

      //if the ball is not within kicking range or there is already a
      //receiving player, this player cannot pass the ball to the player
      //making the request.
      if (player->Team()->Receiver() != NULL || !player->BallWithinKickingRange() )
        return true;

      //make the pass   
      player->Ball()->Kick(receiver->Pos() - player->Ball()->Pos(), 0.4);//MaxPassingForce
      //let the receiver know a pass is coming 
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, player->ID(), receiver->ID(), Msg_ReceiveBall,   receiver->Pos(),
                             &position);
      //change state   
      player->GetFSM()->ChangeState(Wait::Instance());
      player->FindSupport();
      return true;
    }

    break;

  }//end switch

  return false;
}
                                

       

//***************************************************************************** CHASEBALL

ChaseBall* ChaseBall::Instance()
{
  static ChaseBall instance;
  return &instance;
}


void ChaseBall::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("C"));
  player->Steering()->SeekOn();

}

void ChaseBall::Execute(FieldPlayer* player)                                     
{
  //if the ball is within kicking range the player changes state to KickBall.
  if (player->BallWithinKickingRange())
  {
    player->GetFSM()->ChangeState(KickBall::Instance());
    return;
  }
                                                                              
  //if the player is the closest player to the ball then he should keep chasing it
  else if (player->isClosestTeamMemberToBall())
  {
    player->Steering()->SetTarget(player->Ball()->Pos());
    return;
  }
  //if the player is not closest to the ball anymore, he should return back
  //to his home region and wait for another opportunity
  else
    player->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());
}


void ChaseBall::Exit(FieldPlayer* player)
{
  player->Steering()->SeekOff();
}



//*****************************************************************************SUPPORT ATTACKING PLAYER

SupportAttacker* SupportAttacker::Instance()
{
  static SupportAttacker instance;
  return &instance;
}


void SupportAttacker::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("S"));
  player->Steering()->ArriveOn();
  player->Steering()->SetTarget(player->Team()->GetSupportCalcObject()->GetBestSupportingSpot());
}

void SupportAttacker::Execute(FieldPlayer* player)                                     
{


  //if the best supporting spot changes, change the steering target
  if (player->Team()->GetSupportSpot() != player->Steering()->Target())
  {
    player->Steering()->SetTarget(player->Team()->GetSupportSpot());
    player->Steering()->ArriveOn();
  }

    if (player->isClosestTeamMemberToBall() )

    {
        player->GetFSM()->ChangeState(ChaseBall::Instance());
        return;
    }
    //if his team loses control go back home
    else if (!player->Team()->InControl())
    {
        player->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());
        return;
    }
    if (player->AtTarget())
  {
    player->Steering()->ArriveOff();
    //the player should keep his eyes on the ball!
    player->TrackBall();
    player->SetVelocity(Vector2D(0,0));
    //if not threatened by another player request a pass
    if (!player->isThreatened())
      player->Team()->RequestPass(player);
  }
}


void SupportAttacker::Exit(FieldPlayer* player)
{
  //set supporting player to null so that the team knows it has to 
  //determine a new one.
  player->Team()->SetSupportingPlayer(NULL);
  player->Steering()->ArriveOff();
}




//************************************************************************ RETURN TO HOME REGION

ReturnToHomeRegion* ReturnToHomeRegion::Instance()
{
  static ReturnToHomeRegion instance;
  return &instance;
}


void ReturnToHomeRegion::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("H"));
    player->Steering()->ArriveOn();

    if (!player->HomeRegion()->Inside(player->Steering()->Target(), Region::halfsize))
    {
        player->Steering()->SetTarget(player->HomeRegion()->Center());
    }

}

void ReturnToHomeRegion::Execute(FieldPlayer* player)
{
    if(player->Pitch()->GameOn()) // the game is on
    {
        if( player->isClosestTeamMemberToBall() && (player->Team()->Receiver() == NULL) &&
            !player->Pitch()->GoalKeeperHasBall())
        {
            player->GetFSM()->ChangeState(ChaseBall::Instance());
            return;
        }
    }
     if (player->Pitch()->GameOn()&& player->HomeRegion()->Inside(player->Pos(), Region::halfsize))
    {
        player->Steering()->SetTarget(player->Pos());
        player->GetFSM()->ChangeState(Wait::Instance());
    }

  //if game is not on the player must return much closer to the center of his
  //home region
  else if(!player->Pitch()->GameOn() && player->AtTarget())
  {
    player->GetFSM()->ChangeState(Wait::Instance());
  }
}

void ReturnToHomeRegion::Exit(FieldPlayer* player)
{
  player->Steering()->ArriveOff();
}

//***************************************************************************** WAIT

Wait* Wait::Instance()
{
  static Wait instance;
  return &instance;
}

void Wait::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("W"));
  if (!player->Pitch()->GameOn())
  {
    player->Steering()->SetTarget(player->HomeRegion()->Center());
  }
}

void Wait::Execute(FieldPlayer* player) {
    if (!player->AtTarget()) {
        player->Steering()->ArriveOn();
        return;
    }
    else
    {
    player->Steering()->ArriveOff();
    player->SetVelocity(Vector2D(0, 0));
    player->TrackBall();
    }
  //if this player's team is controlling AND this player is not the attacker
  //AND is further up the field than the attacker he should request a pass.
    if (player->Pitch()->GameOn())
    {
        if (/*!player->Team()->InControl() && */player->isClosestTeamMemberToBall()
            && !player->Pitch()->GoalKeeperHasBall() )
        {
            player->GetFSM()->ChangeState(ChaseBall::Instance());
            return;
        }

        else if ( player->Team()->InControl()   && (!player->isControllingPlayer()) )
        {
            player->Team()->RequestPass(player);
            return;
        }
    }
    else{ /*the game is not on - stay put*/ }

}

void Wait::Exit(FieldPlayer* player){

}

//************************************************************************ KICK BALL

KickBall* KickBall::Instance()
{
  static KickBall instance;
  return &instance;
}


void KickBall::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("K"));
  //let the team know this player is controlling
   player->Team()->SetControllingPlayer(player);
   
   //the player can only make so many kick attempts per second.
   if (!player->isReadyForNextKick())
   {
   player->GetFSM()->ChangeState(ChaseBall::Instance());
   }

}

void KickBall::Execute(FieldPlayer* player)
{
  //if a shot is possible, this vector will hold the position along the 
  //opponent's goal line the player should aim for.
   Vector2D    BallTarget;
   float power = 0.43f;//MaxShootingForce

//------------------Attempt a shot at the goal
  if (player->Team()->CanShoot(player->Ball()->Pos(), power, BallTarget))
  {
   //add some noise to the kick. We don't want players who are too accurate!
   BallTarget = AddNoiseToKick(player->Ball()->Pos(), BallTarget);
   //this is the direction the ball will be kicked in
   Vector2D KickDirection = BallTarget - player->Ball()->Pos();
   player->Ball()->Kick(KickDirection, power);
   //change state   
   player->GetFSM()->ChangeState(Wait::Instance());
   player->FindSupport();
   return;
 }
  //---------------- Attempt a pass to a teammate

  //if a receiver is found this will point to it
  PlayerBase* receiver = NULL;
  Vector2D    PassTarget;
  power = 0.4f;
  //test if there are any potential candidates available to receive a pass
  if (player->isThreatened()  && player->Team()->FindPass(player, receiver, PassTarget, power, 0.5f)) //MinPassDist
  {     
    //add some noise to the kick
 //   PassTarget = AddNoiseToKick(player->Ball()->Pos(), PassTarget);
    Vector2D KickDirection = PassTarget - player->Ball()->Pos();
    player->Ball()->Kick(KickDirection, power);
    //let the receiver know a pass is coming 
    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, player->ID(), receiver->ID(), Msg_ReceiveBall,
                            PassTarget, &PassTarget);
    //the player should wait at his current position unless instructed otherwise
    player->GetFSM()->ChangeState(Wait::Instance());
    player->FindSupport();
    return;
  }

  //---------------cannot shoot or pass, so dribble the ball forward
  else
  {   
    player->FindSupport();
    player->GetFSM()->ChangeState(Dribble::Instance());
  }   
}

//*************************************************************************** DRIBBLE

Dribble* Dribble::Instance()
{
  static Dribble instance;
  return &instance;
}


void Dribble::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("D"));
  //let the team know this player is controlling
  player->Team()->SetControllingPlayer(player);
}

void Dribble::Execute(FieldPlayer* player) {
    if (!player->isThreatened()) {
        player->Ball()->Kick(player->Team()->HomeGoal()->Facing(), 0.2f);  //MaxDribbleForce
        //the player has kicked the ball so he must now change state to follow it
        player->GetFSM()->ChangeState(ChaseBall::Instance());
        return;
    }
    else{
        player->GetFSM()->ChangeState(KickBall::Instance());
    }
  return;  
}



//************************************************************************     RECEIVEBALL

ReceiveBall* ReceiveBall::Instance()
{
  static ReceiveBall instance;
  return &instance;
}


void ReceiveBall::Enter(FieldPlayer* player)
{
    player->RecordCurStateName(std::string("R"));
  //let the team know this player is receiving the ball
  player->Team()->SetReceiver(player);
  //this player is also now the controlling player
  player->Team()->SetControllingPlayer(player);
}

void ReceiveBall::Execute(FieldPlayer* player) {
    //if the ball comes close enough to the player or if his team lose control
    //he should change state to chase the ball
    if (player->BallWithinReceivingRange()|| !player->Team()->InControl())
    {
        player->GetFSM()->ChangeState(KickBall::Instance());
        return;
    } else if (player->isClosestTeamMemberToBall() ){//&& player->Ball()->Velocity().LengthSq() < 0.01f) {
        player->Steering()->SetTarget(player->Ball()->Pos());
        player->GetFSM()->ChangeState(ChaseBall::Instance());
        return;
    }
}
void ReceiveBall::Exit(FieldPlayer* player)
{
  player->Team()->SetReceiver(NULL);
}




 



