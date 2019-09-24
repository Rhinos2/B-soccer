#include "TeamStates.h"
#include "SoccerTeam.h"
#include "PlayerBase.h"
#include "../common/MessageDispatcher.h"
#include "SoccerMessages.h"
#include "ConstantsToo.h"
#include "SoccerBall.h"


//defines the size of a team -- do not adjust
const int TeamSize = 4;

void ChangePlayerHomeRegions(SoccerTeam* team, const int NewRegions[TeamSize])
{
  for (int plyr=0; plyr<TeamSize; ++plyr)
  {
    team->SetPlayerHomeRegion(plyr, NewRegions[plyr]);
  }
}

//************************************************************************ ATTACKING

Attacking* Attacking::Instance()
{
  static Attacking instance;
  return &instance;
}


void Attacking::Enter(SoccerTeam* team)
{

  //these define the home regions for this state of each of the players
  const int BlueRegions[TeamSize] = {1,12,14,7};
  const int RedRegions[TeamSize] = {16,5,3,10};

  //set up the player's home regions
  if (team->Color() == SoccerTeam::blue)
    ChangePlayerHomeRegions(team, BlueRegions);
  else
    ChangePlayerHomeRegions(team, RedRegions);

  team->UpdateTargetsOfWaitingPlayers();
}


void Attacking::Execute(SoccerTeam* team) {
    //if this team is no longer in control change states
    if (!team->InControl()) {
    team->GetFSM()->ChangeState(Defending::Instance());
    return;
    }

}

void Attacking::Exit(SoccerTeam* team)
{
  team->SetSupportingPlayer(NULL);
}



//************************************************************************ DEFENDING

Defending* Defending::Instance()
{
  static Defending instance;
  return &instance;
}

void Defending::Enter(SoccerTeam* team)
{
  //these define the home regions for this state of each of the players
  const int BlueRegions[TeamSize] = {1,6,8,4};
  const int RedRegions[TeamSize] = {16,11,9,13};

  //set up the player's home regions
  if (team->Color() == SoccerTeam::blue)
     ChangePlayerHomeRegions(team, BlueRegions);
  else
      ChangePlayerHomeRegions(team, RedRegions);

  team->UpdateTargetsOfWaitingPlayers();
}

void Defending::Execute(SoccerTeam* team)
{
  //if in control change states
  if (team->InControl())
    team->GetFSM()->ChangeState(Attacking::Instance()); return;
}

void Defending::Exit(SoccerTeam* team){

}

//************************************************************************ KICKOFF
PrepareForKickOff* PrepareForKickOff::Instance()
{
  static PrepareForKickOff instance;
  return &instance;
}

void PrepareForKickOff::Enter(SoccerTeam* team)
{
    //these define the home regions for this state of each of the players
    const int BlueRegions[TeamSize] = {1,6,8,4};
    const int RedRegions[TeamSize] = {16,11,9,13};

    //set up the player's home regions
    if (team->Color() == SoccerTeam::blue)
        ChangePlayerHomeRegions(team, BlueRegions);
    else
        ChangePlayerHomeRegions(team, RedRegions);
  //reset key player pointers
  team->SetControllingPlayer(NULL);
  team->SetSupportingPlayer(NULL);
  team->SetReceiver(NULL);
  team->SetPlayerClosestToBall(NULL);
  team->Pitch()->SetGameOff();
  team->ReturnAllFieldPlayersToHome();
   team->Opponents()->ReturnAllFieldPlayersToHome();
}

void PrepareForKickOff::Execute(SoccerTeam* team)
{
  //if both teams in position, start the game
 if (team->AllPlayersAtHome() && team->Opponents()->AllPlayersAtHome())
 {
     static int count = 0; //pause at the kickoff
     count++;
     if(count > 30) {
         count = 0;
         team->GetFSM()->ChangeState(Defending::Instance());
     }
 }
 else {
   ;
 }
}

void PrepareForKickOff::Exit(SoccerTeam* team)
{
  team->Pitch()->SetGameOn();
}


