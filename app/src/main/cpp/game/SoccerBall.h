#ifndef SOCCERBALL_H
#define SOCCERBALL_H


#include <vector>
#include "../common/MovingEntity.h"


class Wall2D;
class PlayerBase;


class SoccerBall : public MovingEntity
{
private:

  //keeps a record of the ball's position at the last update
  Vector2D                  m_vOldPos;

  //a local reference to the Walls that make up the pitch boundary
  const std::vector<Wall2D>& m_PitchBoundary;                                      


public:
    //tests to see if the ball has collided with a wall and reflects
  //the ball's velocity accordingly
  void TestCollisionWithWalls(const std::vector<Wall2D>& walls);

  SoccerBall(Vector2D  pos, float BallSize, float mass, std::vector<Wall2D>& PitchBoundary):
  
      //set up the base class
      MovingEntity(pos, BallSize, Vector2D(0.0f,0.0f), -1.0f,     //max speed - unused
                  Vector2D(0,1), mass,
                  Vector2D(1.0f,1.0f),  //scale     - unused
                  0,                   //turn rate - unused
                  0),                  //max force - unused
     m_PitchBoundary(PitchBoundary)
  {}
  
  //implement base class Update
  void      Update();

  //a soccer ball doesn't need to handle messages
  bool      HandleMessage(const Telegram& msg){return false;}

  //this method applies a directional force to the ball (kicks it!)
  void      Kick(Vector2D direction, float force);

  //given a kicking force and a distance to traverse defined by start
  //and finish points, this method calculates how long it will take the
  //ball to cover the distance.
  float    TimeToCoverDistance(Vector2D from,
                               Vector2D to,
                               float     force)const;

  //this method calculates where the ball will in 'time' seconds
  Vector2D FuturePosition(float time)const;

  //this is used by players and goalkeepers to 'trap' a ball -- to stop
  //it dead. That player is then assumed to be in possession of the ball
  //and m_pOwner is adjusted accordingly
  void      Trap(){m_vVelocity.Zero();}  

  Vector2D  OldPos()const{return m_vOldPos;}
  
  //this places the ball at the desired location and sets its velocity to zero
  void      PlaceAtPosition(Vector2D NewPos);
};



//this can be used to vary the accuracy of a player's kick.
Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget);



#endif