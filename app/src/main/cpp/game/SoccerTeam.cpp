#include "SoccerTeam.h"
#include "Goal.h"
#include "PlayerBase.h"
#include "Goalkeeper.h"
#include "FieldPlayer.h"
#include "../common/utils.h"
#include "SteeringBehaviors.h"
#include "GoalKeeperStates.h"
#include "../common/geometry.h"
#include "EntityManager.h"
#include "../common/MessageDispatcher.h"
#include "SoccerMessages.h"
#include "TeamStates.h"


using std::vector;


//----------------------------- ctor -------------------------------------
//
//------------------------------------------------------------------------
SoccerTeam::SoccerTeam(Goal*  home_goal,
                       Goal*  opponents_goal,
                       GameApp* pitch,
                       team_color   color):m_pOpponentsGoal(opponents_goal),
                                           m_pHomeGoal(home_goal),
                                           m_pOpponents(NULL),
                                           m_pPitch(pitch),
                                           m_Color(color),
                                           m_dDistSqToBallOfClosestPlayer(0.0f),
                                           m_pSupportingPlayer(NULL),
                                           m_pReceivingPlayer(NULL),
                                           m_pControllingPlayer(NULL),
                                           m_pPlayerClosestToBall(NULL)
{
  //setup the state machine
  m_pStateMachine = new StateMachine<SoccerTeam>(this);

  m_pStateMachine->SetCurrentState(Defending::Instance());
  m_pStateMachine->SetPreviousState(Defending::Instance());
  m_pStateMachine->SetGlobalState(NULL);
  //create the players and goalkeeper
  CreatePlayers();
  //set default steering behaviors
  for (std::vector<PlayerBase*>::iterator it = m_Players.begin(); it != m_Players.end(); ++it)
  {
    (*it)->Steering()->SeparationOn();
  }
  //create the support spot calculator
  m_pSupportSpotCalc = new SupportSpotCalculator(4,/*NumSupportSpotsX*/ 8,/*NumSupportSpotsZ*/ this);
}


SoccerTeam::~SoccerTeam()
{
  delete m_pStateMachine;

  std::vector<PlayerBase*>::iterator it = m_Players.begin();
  for (it; it != m_Players.end(); ++it)
  {
    delete *it;
  }

  delete m_pSupportSpotCalc;
}

//-------------------------- update --------------------------------------
//
//  iterates through each player's update function and calculates 
//  frequently accessed info
//------------------------------------------------------------------------
void SoccerTeam::Update()
{
  //this information is used frequently so it's more efficient to 
  //calculate it just once each frame
  CalculateClosestPlayerToBall();

  //the team state machine switches between attack/defense behavior. It
  //also handles the 'kick off' state where a team must return to their
  //kick off positions before the whistle is blown
  m_pStateMachine->Update();
  
  //now update each player
  std::vector<PlayerBase*>::iterator it = m_Players.begin();

  for (it; it != m_Players.end(); ++it)
  {
    (*it)->Update();
  }

}


//------------------------ CalculateClosestPlayerToBall ------------------
//
//  sets m_iClosestPlayerToBall to the player closest to the ball
//------------------------------------------------------------------------
void SoccerTeam::CalculateClosestPlayerToBall()
{
  float ClosestSoFar = 12.0f * 12.0f;//Sqd pitch length

  for (std::vector<PlayerBase*>::iterator it = m_Players.begin(); it != m_Players.end(); ++it)
  {
    //calculate the dist. Use the squared value to avoid sqrt
    float dist = Vec2DDistanceSq((*it)->Pos(), Pitch()->Ball()->Pos());

    //keep a record of this value for each player
    (*it)->SetDistSqToBall(dist);
    
    if (dist < ClosestSoFar)
    {
      ClosestSoFar = dist;
      m_pPlayerClosestToBall = *it;
    }
  }

  m_dDistSqToBallOfClosestPlayer = ClosestSoFar;
}


//------------- DetermineBestSupportingAttacker ------------------------
//
// calculate the closest player to the SupportSpot
//------------------------------------------------------------------------
PlayerBase* SoccerTeam::DetermineBestSupportingAttacker()
{
  float ClosestSoFar = MaxFloat;
  PlayerBase* BestPlayer = NULL;

  for (std::vector<PlayerBase*>::iterator it = m_Players.begin(); it != m_Players.end(); ++it)
  {
    //only players not currently controlling the ball utilize the BestSupportingSpot
    if ((*it) != m_pControllingPlayer)
    {
      //calculate the dist. Use the squared value to avoid sqrt
      float dist = Vec2DDistanceSq((*it)->Pos(), m_pSupportSpotCalc->GetBestSupportingSpot());
      //keep a record of this player
      if ((dist < ClosestSoFar) )
      {
        ClosestSoFar = dist;
        BestPlayer = (*it);
      }
    }
  }
  return BestPlayer;
}

//-------------------------- FindPass ------------------------------
//
//  The best pass is considered to be the pass that cannot be intercepted 
//  by an opponent and that is as far forward of the receiver as possible
//------------------------------------------------------------------------
bool SoccerTeam::FindPass(const PlayerBase*const passer, PlayerBase*& receiver, Vector2D& PassTarget,
                         float power, float MinPassingDistance)const
{
  //     FILE* stream;
  //    char filenamepath[] = "/sdcard/Music/mydumpobj.txt";
   //  char error[128];
   //   char* path;

    // if( (stream = fopen(filenamepath, "a")) == NULL) {
    // path = strerror(errno);
    // strcpy(error, path);
     //   exit(2);
    // }
  float    ClosestToGoalSoFar = MaxFloat;
  Vector2D Target;

  for (std::vector<PlayerBase*>::const_iterator curPlyr = Members().begin(); curPlyr != Members().end(); curPlyr++)
  {
      if(passer->ID() == (*curPlyr)->ID()) continue; //passer itself
    //  if(passer->ID() == 1 || passer->ID() == 6) continue; //keepers
      if((Vec2DDistanceSq(passer->Pos(), (*curPlyr)->Pos()) < MinPassingDistance*MinPassingDistance))
          continue;

      if (GetBestPassToReceiver(passer, *curPlyr, Target, power))
      {

      Target = (*curPlyr)->Pos();
        //if the pass target is the closest to the opponent's goal line found
        // so far, keep a record of it
        float Dist2Goal = fabs(Target.y - OpponentsGoal()->Center().y);
        if (Dist2Goal < ClosestToGoalSoFar) {
            ClosestToGoalSoFar = Dist2Goal;
            //keep a record of this player
            receiver = *curPlyr;
            //and the target
            PassTarget = Target;
             //fprintf(stream, "PassTarget X: %f Y: %f\n", PassTarget.x, PassTarget.y);
            // fclose(stream);
            }
        }     

  }//next team member

  if (receiver) return true;
 
  else return false;
}


//---------------------- GetBestPassToReceiver ---------------------------
//
//------------------------------------------------------------------------
bool SoccerTeam::GetBestPassToReceiver( const PlayerBase*  passer,  const PlayerBase*  receiver,
                                       Vector2D&  PassTarget, const float power) const
{

    //first, calculate how much time it will take for the ball to reach
  //this receiver, if the receiver was to remain motionless 
  float time = Pitch()->Ball()->TimeToCoverDistance(Pitch()->Ball()->Pos(), receiver->Pos(), power);
  if (time < 0) return false;
  //the maximum distance the receiver can cover in this time
  float InterceptRange = time * receiver->MaxSpeed();
  
  //Scale the intercept range
  const float ScalingFactor = 0.2;
  InterceptRange *= ScalingFactor;

  //now calculate the pass targets which are positioned at the intercepts
  //of the tangents from the ball to the receiver's range circle.
  Vector2D ip1, ip2;

  GetTangentPoints(receiver->Pos(), InterceptRange, Pitch()->Ball()->Pos(), ip1, ip2);

  const int NumPassesToTry = 3;
  Vector2D Passes[NumPassesToTry] = {ip1, receiver->Pos(), ip2};

  float ClosestSoFar = MaxFloat;
  bool  bResult      = false;

  for (int pass=0; pass<NumPassesToTry; ++pass) {
      float dist = fabs(Passes[pass].y - OpponentsGoal()->Center().y);
      if ((dist < ClosestSoFar) && Pitch()->PlayingArea()->Inside(Passes[pass], Region::normal))
      {
          ClosestSoFar = dist;
          PassTarget = Passes[pass];
          bResult = true;
      }
  }
  return bResult;
}

//----------------------- isPassSafeFromOpponent -------------------------
//
//  test if a pass from 'from' to 'to' can be intercepted by an opposing
//  player
//------------------------------------------------------------------------
bool SoccerTeam::isPassSafeFromOpponent(Vector2D    from, Vector2D    target, const PlayerBase* const receiver,
                                        const PlayerBase* const opp, float  PassingForce)const
{
  //move the opponent into local space.
  Vector2D ToTarget = target - from;
  Vector2D ToTargetNormalized = Vec2DNormalize(ToTarget);
Vector2D Perp (-ToTargetNormalized.y, ToTargetNormalized.x);
  Vector2D LocalPosOpp = PointToLocalSpace(opp->Pos(), ToTargetNormalized, Perp,//ToTargetNormalized.Perp(),
                                         from);

  //if opponent is behind the kicker then pass is considered okay(this is 
  //based on the assumption that the ball is going to be kicked with a 
  //velocity greater than the opponent's max velocity)
  if ( LocalPosOpp.x < 0 )
  {     
    return true;
  }
  
  //if the opponent is further away than the target we need to consider if
  //the opponent can reach the position before the receiver.
  if (Vec2DDistanceSq(from, target) < Vec2DDistanceSq(opp->Pos(), from))
  {
    if (receiver)
    {
      if ( Vec2DDistanceSq(target, opp->Pos())  > Vec2DDistanceSq(target, receiver->Pos()) )
          return true;
      else
        return false;
    }
    else
      return true;
  }
  
  //calculate how long it takes the ball to cover the distance to the 
  //position orthogonal to the opponents position
  float TimeForBall = 
  Pitch()->Ball()->TimeToCoverDistance(Vector2D(0,0), Vector2D(LocalPosOpp.x, 0), PassingForce);

  //now calculate how far the opponent can run in this time
  float reach = opp->MaxSpeed() * TimeForBall + Pitch()->Ball()->BRadius()+ opp->BRadius();

  //if the distance to the opponent's y position is less than his running
  //range plus the radius of the ball and the opponents radius then the
  //ball can be intercepted
  if ( fabs(LocalPosOpp.y) < reach )
  {
    return false;
  }

  return true;
}

//---------------------- isPassSafeFromAllOpponents ----------------------
//
//  tests a pass from position 'from' to position 'target' against each member
//  of the opposing team. Returns true if the pass can be made without
//  getting intercepted
//------------------------------------------------------------------------
bool SoccerTeam::isPassSafeFromAllOpponents(Vector2D from, Vector2D target,
                  const PlayerBase* const receiver, float PassingForce)const
{
  std::vector<PlayerBase*>::const_iterator opp = Opponents()->Members().begin();
  for (opp; opp != Opponents()->Members().end(); ++opp)
  {
    if (!isPassSafeFromOpponent(from, target, receiver, *opp, PassingForce))
        return false;
  }
  return true;
}

//------------------------ CanShoot --------------------------------------
//
//  Given a ball position, a kicking power and a reference to a vector2D
//  this function will sample random positions along the opponent's goal-
//  mouth and check to see if a goal can be scored if the ball was to be
//  kicked in that direction with the given power. If a possible shot is 
//  found, the function will immediately return true, with the target 
//  position stored in the vector ShotTarget.
//------------------------------------------------------------------------
bool SoccerTeam::CanShoot(Vector2D  BallPos, float power, Vector2D& ShotTarget)const
{
    ShotTarget = OpponentsGoal()->Center();
    //it makes little sense to try to score from half the field's length
    Vector2D dist = ShotTarget - BallPos;
    Region* PitchArea = Pitch()->PlayingArea();
    float halfPitchSqr = PitchArea->Height() * PitchArea->Height() / 4; //36.0
    if (halfPitchSqr <= dist.LengthSq() + 25.0f) //<= 16.0 + 25.0
        return false;
  //the number of randomly created shot targets this method will test 
  int NumAttempts = 3;
  while (NumAttempts--)
  {
    float randOffset = RandInRange(OpponentsGoal()->LeftPost().x, OpponentsGoal()->RightPost().x);
    ShotTarget.x += randOffset;
    //make sure striking the ball with the given power is enough to drive the ball over the goal line.
    float time = Pitch()->Ball()->TimeToCoverDistance(BallPos, ShotTarget, power);

    if (time >= 0)
        return true;
  }
  
  return false;
}

//--------------------- ReturnAllFieldPlayersToHome ---------------------------
//
//  sends a message to all players to return to their home areas forthwith
//------------------------------------------------------------------------
void SoccerTeam::ReturnAllFieldPlayersToHome()const
{
  for (std::vector<PlayerBase*>::const_iterator it = m_Players.begin(); it != m_Players.end(); ++it)
  {
    if ((*it)->Role() != PlayerBase::goal_keeper)
    {
      Vector2D place_gohome;
        (*it)->SetDefaultHomeRegion();
        ((FieldPlayer*)(*it))->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());
    }
  }
}

//------------------------- CreatePlayers --------------------------------
//
//  creates the players
//------------------------------------------------------------------------
void SoccerTeam::CreatePlayers()
{
  if (Color() == blue)
  {
    //goalkeeper
    m_Players.push_back(new GoalKeeper(this, 1, TendGoal::Instance(), Vector2D(0,1), Vector2D(0.0, 0.0),
		5.0f,//PlayerMass
		3.0f,//PlayerMaxForce
		0.1f,//PlayerMaxSpeedWithoutBall
		0.8f,//PlayerMaxTurnRate
		1.0f,
    std::string("T")));

    //create the players
    m_Players.push_back(new FieldPlayer(this, 6, Wait::Instance(), Vector2D(0,1), Vector2D(0.0, 0.0),
		5.0f,//PlayerMass
		3.0f,//PlayerMaxForce
        0.1f,//PlayerMaxSpeedWithoutBall
		0.8f,//PlayerMaxTurnRate
		1.0f,//PlayerScale
        PlayerBase::attacker, std::string("W")));



       m_Players.push_back(new FieldPlayer(this, 8, Wait::Instance(), Vector2D(0,1), Vector2D(0.0, 0.0),
			5.0f,//PlayerMass
			3.0f,//PlayerMaxForce
			0.1f,//PlayerMaxSpeedWithoutBall
			0.8f,//PlayerMaxTurnRate
			1.0f,//PlayerScale
            PlayerBase::attacker, std::string("W")));


 


        m_Players.push_back(new FieldPlayer(this, 4, Wait::Instance(), Vector2D(0,1), Vector2D(0.0, 0.0),
			5.0f,//PlayerMass
			3.0f,//PlayerMaxForce
			0.1f,//PlayerMaxSpeedWithoutBall
			0.8f,//PlayerMaxTurnRate
			1.0f,//PlayerScale//
             PlayerBase::defender, std::string("W")));

  }

  else
  {

     //goalkeeper
    m_Players.push_back(new GoalKeeper(this, 16, TendGoal::Instance(), Vector2D(0,-1), Vector2D(0.0, 0.0),
		5.0f,//PlayerMass
		3.0f,//PlayerMaxForce
		0.1f,//PlayerMaxSpeedWithoutBall
		0.8f,//PlayerMaxTurnRate
		1.0f,
        std::string("T")));//PlayerScale


    //create the players
    m_Players.push_back(new FieldPlayer(this, 11, Wait::Instance(), Vector2D(0,-1), Vector2D(0.0, 0.0),
		5.0f,//PlayerMass
		3.0f,//PlayerMaxForce
		0.1f,//PlayerMaxSpeedWithoutBall
		0.8f,//PlayerMaxTurnRate
		1.0f,//PlayerScale
      PlayerBase::attacker, std::string("W")));

    m_Players.push_back(new FieldPlayer(this,9, Wait::Instance(), Vector2D(0,-1), Vector2D(0.0, 0.0),
                               5.0f,//PlayerMass
                               3.0f,//PlayerMaxForce
                               0.1f,//PlayerMaxSpeedWithoutBall
                               0.8f,//PlayerMaxTurnRate
                               1.0f,//PlayerScale
                               PlayerBase::attacker, std::string("W")));


 
    m_Players.push_back(new FieldPlayer(this, 13, Wait::Instance(), Vector2D(0,-1), Vector2D(0.0, 0.0),
                               5.0f,//PlayerMass
                               3.0f,//PlayerMaxForce
                               0.1f,//PlayerMaxSpeedWithoutBall
                               0.8f,//PlayerMaxTurnRate
                               1.0f,//PlayerScale
                               PlayerBase::defender, std::string("W")));

  }

  for (std::vector<PlayerBase*>::iterator it = m_Players.begin(); it != m_Players.end(); ++it)
  {
    EntityMgr->RegisterEntity(*it);
  }
}


PlayerBase* SoccerTeam::GetPlayerFromID(int id)const
{
  std::vector<PlayerBase*>::const_iterator it = m_Players.begin();

  for (it; it != m_Players.end(); ++it)
  {
    if ((*it)->ID() == id) return *it;
  }

  return NULL;
}


void SoccerTeam::SetPlayerHomeRegion(int plyr, int region)const
{
  assert ( (plyr>=0) && (plyr<m_Players.size()) );

  m_Players[plyr]->SetHomeRegion(region);
}


//---------------------- UpdateTargetsOfWaitingPlayers ------------------------
//
//  
void SoccerTeam::UpdateTargetsOfWaitingPlayers()const
{
  for (std::vector<PlayerBase*>::const_iterator it = m_Players.begin(); it != m_Players.end(); ++it)
  {  
    if ( (*it)->Role() != PlayerBase::goal_keeper )
    {
      FieldPlayer* plyr = static_cast<FieldPlayer*>(*it);
      if ( plyr->GetFSM()->isInState(*Wait::Instance())|| plyr->GetFSM()->isInState(*ReturnToHomeRegion::Instance())){
          plyr->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());
         //  plyr->GetFSM()->isInState(*ReturnToHomeRegion::Instance()) )
            plyr->Steering()->SetTarget(plyr->HomeRegion()->Center());
      }
    }
  }
}


//--------------------------- AllPlayersAtHome --------------------------------
//
//  returns false if any of the team are not located within their home region
//-----------------------------------------------------------------------------
bool SoccerTeam::AllPlayersAtHome()const
{
  for (std::vector<PlayerBase*>::const_iterator it = m_Players.begin();it != m_Players.end(); ++it)
  {
    if (!(*it)->InHomeRegion())
      return false;
  }
  return true;
}

//------------------------- RequestPass ---------------------------------------
//
//  this tests to see if a pass is possible between the requester and
//  the controlling player. If it is possible a message is sent to the
//  controlling player to pass the ball asap.
//-----------------------------------------------------------------------------
void SoccerTeam::RequestPass(FieldPlayer* requester)const
{
  
  if (isPassSafeFromAllOpponents(ControllingPlayer()->Pos(), requester->Pos(), requester,
                                 0.4f))//MaxPassingForce
  {

    //tell the player to make the pass //let the receiver know a pass is coming
    Vector2D place_passtome;
    place_passtome.x = requester->Pos().x;
    place_passtome.y = requester->Pos().y;

    Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, requester->ID(), ControllingPlayer()->ID(),
                          Msg_PassToMe, place_passtome, requester);

  }
}


//----------------------------- isOpponentWithinRadius ------------------------
//
//  returns true if an opposing player is within the radius of the position
//  given as a parameter
//-----------------------------------------------------------------------------
bool SoccerTeam::isOpponentWithinRadius(Vector2D pos, float rad)
{
  std::vector<PlayerBase*>::const_iterator end = Opponents()->Members().end();
  std::vector<PlayerBase*>::const_iterator it;

  for (it=Opponents()->Members().begin(); it !=end; ++it)
  {
    if (Vec2DDistanceSq(pos, (*it)->Pos()) < rad*rad)
    {
      return true;
    }
  }

  return false;
}
