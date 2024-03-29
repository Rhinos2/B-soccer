#include <common/Transformations.h>
#include "SoccerBall.h"
#include "../common/geometry.h"
//#include "../Common/Debug/DebugConsole.h"
//#include "../Common/misc/Cgdi.h"
//#include "ParamLoader.h"
#include "../common/Wall2D.h"
#include "../common/utils.h"


//----------------------------- AddNoiseToKick --------------------------------
//
//  this can be used to vary the accuracy of a player's kick. Just call it 
//  prior to kicking the ball using the ball's position and the ball target as
//  parameters.
//-----------------------------------------------------------------------------
Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget)
{

  float displacement = (Pi - Pi* 0.95f/*(*ParamLoader::Instance()).PlayerKickingAccuracy)*/) * RandomClamped();

  Vector2D toTarget = BallTarget - BallPos;

  Vec2DRotateAroundOrigin(toTarget, displacement);

  return toTarget + BallPos;
}

  

//-------------------------- Kick ----------------------------------------
//                                                                        
//  applys a force to the ball in the direction of heading. Truncates
//  the new velocity to make sure it doesn't exceed the max allowable.
//------------------------------------------------------------------------
void SoccerBall::Kick(Vector2D direction, float force)
{  
  //ensure direction is normalized
  direction.Normalize();
  
  //calculate the acceleration
  Vector2D acceleration = (direction * force) / m_dMass ;

  //update the velocity
  m_vVelocity = acceleration;
}

//----------------------------- Update -----------------------------------
//
//  updates the ball physics, tests for any collisions and adjusts
//  the ball's velocity accordingly
//------------------------------------------------------------------------
void SoccerBall::Update()
{
  //keep a record of the old position so the goal::scored method
  //can utilize it for goal testing
  static int count = 0;
  count++;
  if(count > 5) {
      m_vOldPos = m_vPosition;
      count = 0;
  }

    TestCollisionWithWalls(m_PitchBoundary);

  //Simulate (*ParamLoader::Instance()).Friction. Make sure the speed is positive 
  //first though
  if (m_vVelocity.LengthSq() > -0.005f * -0.005f)//Friction
  {
    m_vVelocity += Vec2DNormalize(m_vVelocity) * -0.005f;
      m_vVelocity.Truncate(0.2f);
    m_vPosition += m_vVelocity;

    //update heading
    m_vHeading = Vec2DNormalize(m_vVelocity);
  }
}

//---------------------- TimeToCoverDistance -----------------------------
//
//  Given a force and a distance to cover given by two vectors, this
//  method calculates how long it will take the ball to travel between
//  the two points
//------------------------------------------------------------------------
float SoccerBall::TimeToCoverDistance(Vector2D A, Vector2D B, float force)const
{
  //this will be the velocity of the ball in the next time step *if*
  //the player was to make the pass. 
  float speed = force / m_dMass ;

  //calculate the velocity at B using the equation
  //
  //  v^2 = u^2 + 2as
  //

  //first calculate s (the distance between the two positions)
  float DistanceToCover =  Vec2DDistance(A, B);

  float term = speed*speed + 2.0f*DistanceToCover*-0.005f; //Friction

  //if  (u^2 + 2as) is negative it means the ball cannot reach point B.
  if (term <= 0.0f) return -1.0f;

  float v = sqrt(term);

  //it IS possible for the ball to reach B and we know its speed when it
  //gets there, so now it's easy to calculate the time using the equation
  //
  //    t = v-u
  //        ---
  //         a
  //
  return (v-speed)/-0.005f;//Friction
}

//--------------------- FuturePosition -----------------------------------
//
//  given a time this method returns the ball position at that time in the
//  future
//------------------------------------------------------------------------
Vector2D SoccerBall::FuturePosition(float time)const
{
  //using the equation s = ut + 1/2at^2, where s = distance, a = friction
  //u=start velocity

  //calculate the ut term, which is a vector
  Vector2D ut = m_vVelocity * time;

  //calculate the 1/2at^2 term, which is scalar
  float half_a_t_squared =  0.5f * -0.005f * time * time;//Friction

  //turn the scalar quantity into a vector by multiplying the value with
  //the normalized velocity vector (because that gives the direction)
  Vector2D ScalarToVector = half_a_t_squared * Vec2DNormalize(m_vVelocity);

  //the predicted position is the balls position plus these two terms
   Vector2D ret = Pos() + ut + ScalarToVector;
   if(ret.y > 6.0f) ret.y = 6.0f;
   else if(ret.y < -6.0f) ret.y = -6.0f;
    if(ret.x > 3.0f) ret.x = 3.0f;
    else if(ret.x < -3.0f) ret.x = -3.0f;
  return ret;
}


//----------------------- TestCollisionWithWalls -------------------------
//
void SoccerBall::TestCollisionWithWalls(const std::vector<Wall2D>& walls)
{  
  //test ball against each wall, find out which is closest
  int idxClosest = -1;

  Vector2D VelNormal = Vec2DNormalize(m_vVelocity);

  Vector2D IntersectionPoint, CollisionPoint;

  float DistToIntersection = MaxFloat;

  //iterate through each wall and calculate if the ball intersects.
  //If it does then store the index into the closest intersecting wall
  for (unsigned int w=0; w<walls.size(); ++w)
  {
    //assuming a collision if the ball continued on its current heading 
    //calculate the point on the ball that would hit the wall. This is 
    //simply the wall's normal(inversed) multiplied by the ball's radius
    //and added to the balls center (its position)
    Vector2D ThisCollisionPoint = Pos() - (walls[w].Normal() * BRadius());

    //calculate exactly where the collision point will hit the plane    
    if (WhereIsPoint(ThisCollisionPoint, walls[w].From(), walls[w].Normal()) == plane_backside)
    {
      float DistToWall = DistanceToRayPlaneIntersection(ThisCollisionPoint,
                                                         walls[w].Normal(),
                                                         walls[w].From(),
                                                         walls[w].Normal());

      IntersectionPoint = ThisCollisionPoint + (DistToWall * walls[w].Normal());
      
    }

    else
    {
      float DistToWall = DistanceToRayPlaneIntersection(ThisCollisionPoint,
                                                         VelNormal,
                                                         walls[w].From(),
                                                         walls[w].Normal());

      IntersectionPoint = ThisCollisionPoint + (DistToWall * VelNormal);
    }
    
    //check to make sure the intersection point is actually on the line
    //segment
    bool OnLineSegment = false;

    if (LineIntersection2D(walls[w].From(), 
                           walls[w].To(),
                           ThisCollisionPoint - walls[w].Normal(),  //*20.0,
                           ThisCollisionPoint + walls[w].Normal()))  //*20.0))
    {

      OnLineSegment = true;                                               
    }

    //Note, there is no test for collision with the end of a line segment
    
    //now check to see if the collision point is within range of the
    //velocity vector. [work in distance squared to avoid sqrt] and if it
    //is the closest hit found so far. 
    //If it is that means the ball will collide with the wall sometime
    //between this time step and the next one.
    float distSq = Vec2DDistanceSq(ThisCollisionPoint, IntersectionPoint);

    if ((distSq <= m_vVelocity.LengthSq()) && (distSq < DistToIntersection) && OnLineSegment)            
    {        
      DistToIntersection = distSq;
      idxClosest = w;
      CollisionPoint = IntersectionPoint;
    }     
  }//next wall

    
  //to prevent having to calculate the exact time of collision we
  //can just check if the velocity is opposite to the wall normal
  //before reflecting it. This prevents the case where there is overshoot
  //and the ball gets reflected back over the line before it has completely
  //reentered the playing area.
  if ( (idxClosest >= 0 ) && VelNormal.Dot(walls[idxClosest].Normal()) < 0)
  {
    m_vVelocity.Reflect(walls[idxClosest].Normal());   
  }
}

//----------------------- PlaceAtLocation -------------------------------------
//
//  positions the ball at the desired location and sets the ball's velocity to
//  zero
//-----------------------------------------------------------------------------
void SoccerBall::PlaceAtPosition(Vector2D NewPos)
{
  m_vPosition = NewPos;

  m_vOldPos = m_vPosition;
  
  m_vVelocity.Zero();
}

