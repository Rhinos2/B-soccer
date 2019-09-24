//
//
//

#ifndef SOCCERSIM_UTILS_H
#define SOCCERSIM_UTILS_H

#include <assert.h>
#include <math.h>
#include <stdlib.h>


const float   Pi        = 3.14159;
const float   TwoPi     = Pi * 2;
const float   HalfPi    = Pi / 2;
const float   QuarterPi = Pi / 4;
//----------------------------------------------------------------------------
//  some random number functions.
//----------------------------------------------------------------------------

//returns a random integer between x and y
inline int   RandInt(int x,int y) {return rand()%(y-x+1)+x;}

//returns a random float between zero and 1
inline float RandFloat()      {return ((rand())/(RAND_MAX+1.0f));}

inline float RandInRange(float x, float y)
{
    return x + RandFloat()*(y-x);
}

//returns a random bool
inline bool   RandBool()
{
    if (RandInt(0,1)) return true;

    else return false;
}

//returns a random float in the range -1 < n < 1
inline float RandomClamped()    {return RandFloat() - RandFloat();}


//returns a random number with a normal distribution. See method at
//http://www.taygeta.com/random/gaussian.html
inline float RandGaussian(float mean = 0.0, float standard_deviation = 1.0)
{
    float x1, x2, w, y1;
    static float y2;
    static int use_last = 0;

    if (use_last)		        /* use value from previous call */
    {
        y1 = y2;
        use_last = 0;
    }
    else
    {
        do
        {
            x1 = 2.0f * RandFloat() - 1.0f;
            x2 = 2.0f * RandFloat() - 1.0f;
            w = x1 * x1 + x2 * x2;
        }
        while ( w >= 1.0 );

        w = sqrt( (-2.0f * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;
        use_last = 1;
    }

    return( mean + y1 * standard_deviation );
}

//returns the maximum of two values
template <class T>
inline T MaxOf(const T& a, const T& b)
{
    if (a>b) return a; return b;
}

//returns the minimum of two values
template <class T>
inline T MinOf(const T& a, const T& b)
{
    if (a<b) return a; return b;
}

//clamps the first argument between the second two
template <class T, class U, class V>
inline void Clamp(T& arg, const U& minVal, const V& maxVal)
{
    assert ( (minVal < maxVal) && "<Clamp>MaxVal < MinVal!");

    if (arg < (T)minVal)
    {
        arg = (T)minVal;
    }

    if (arg > (T)maxVal)
    {
        arg = (T)maxVal;
    }
}

//compares two real numbers. Returns true if they are equal
inline bool isEqual(float a, float b)
{
    if (fabs(a-b) < 1E-12)
        return true;

    return false;
}


#endif
