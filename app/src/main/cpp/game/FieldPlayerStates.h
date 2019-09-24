#ifndef FIELDPLAYERSTATES_H
#define FIELDPLAYERSTATES_H


#include <string>

#include "State.h"
#include "../common/Telegram.h"


class FieldPlayer;
class SoccerPitch;


//------------------------------------------------------------------------
class GlobalPlayerState : public State<FieldPlayer>
{
private:
  
  GlobalPlayerState(){}

public:

  //this is a singleton
  static GlobalPlayerState* Instance();

  void Enter(FieldPlayer* player){}

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player){}

  bool OnMessage(FieldPlayer*, const Telegram&);
};

//------------------------------------------------------------------------
class ChaseBall : public State<FieldPlayer>
{
private:
  
  ChaseBall(){}

public:

  //this is a singleton
  static ChaseBall* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class Dribble : public State<FieldPlayer>
{
private:
  
  Dribble(){}

public:

  //this is a singleton
  static Dribble* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player){}

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};


//------------------------------------------------------------------------
class ReturnToHomeRegion: public State<FieldPlayer>
{

private:
  
  ReturnToHomeRegion(){}

public:

  //this is a singleton
  static ReturnToHomeRegion* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class Wait: public State<FieldPlayer>
{
    bool bChaseOn;
private:
  
  Wait(){bChaseOn = false;}

public:

  //this is a singleton
  static Wait* Instance();
  void Enter(FieldPlayer* player);
  void Execute(FieldPlayer* player);
  void Exit(FieldPlayer* player);
  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
  void ToggleChaseOn(){ if(bChaseOn == true) bChaseOn = false; else bChaseOn = true;}
  void SetChaseOn() { bChaseOn = true; }
};

//------------------------------------------------------------------------
class KickBall: public State<FieldPlayer>
{
private:
  
  KickBall(){}

public:

  //this is a singleton
  static KickBall* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player){}

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};

//------------------------------------------------------------------------
class ReceiveBall: public State<FieldPlayer>
{
private:
  
  ReceiveBall(){}

public:

  //this is a singleton
  static ReceiveBall* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};


//------------------------------------------------------------------------
class SupportAttacker: public State<FieldPlayer>
{
private:
  
  SupportAttacker(){}

public:

  //this is a singleton
  static SupportAttacker* Instance();

  void Enter(FieldPlayer* player);

  void Execute(FieldPlayer* player);

  void Exit(FieldPlayer* player);

  bool OnMessage(FieldPlayer*, const Telegram&){return false;}
};




  
#endif