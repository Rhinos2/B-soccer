#ifndef _SIM_DROIDSOCCER_H_
#define _SIM_DROIDSOCCER_H_

#include <game/Goal.h>
#include <game/SoccerTeam.h>
#include "ActivityHandler.h"

#include "EventLoop.h"
#include "GraphicsManager.h"

#include "Types.h"
#include "AssetReader.h"
#include "0ldFont.h"
#include "GLMath.h"
#include "MyOBJ.h"
#include "ConstantsToo.h"
#include "./common/Wall2D.h"

class Region;
class SoccerTeam;
class Vehicle;

class GameApp  : public ActivityHandler{
public:
    GameApp(android_app* pApplication);
    void run();
    SoccerBall*const Ball()const        {return m_pBall;}
    Region*  const PlayingArea()const  {return m_pPlayingArea;}
    bool  GoalKeeperHasBall()const      {return m_bGoalKeeperHasBall;}
    void  SetGoalKeeperHasBall(bool b)  {m_bGoalKeeperHasBall = b;}
    bool  GameOn()const{return m_bGameOn;}
    void  SetGameOn(){m_bGameOn = true;}
    void  SetGameOff(){m_bGameOn = false;}
    const Region* const GetRegionFromIndex(int idx)
    {
        assert ( (idx > 0) && (idx < m_Regions.size()) );
        return m_Regions[idx];
    }
    //--------------------------------------//

    void UpdateScore(int red, int blue);
    void DrawPitch();
    void RenderTeams();
    void DrawBall();
 //   void ScoreFlagTrue() { m_bScoreFlag = true;}
    GLint viewport_matrix[4];
    SoccerBall* GetBallPtr(){ return m_pBall;}

private:
    GameApp(const GameApp&);
    void operator=(const GameApp&);
//this instantiates the regions the players utilize to  position themselves
    void CreateRegions(float width, float height);
    GLshort CreateShaderProgs();
    void Uniforms(MYPROGRAM* prg, Shaders);
    GraphicsManager mGraphicsManager;
    InputHandler    mInputHandler;

    EventLoop mEventLoop;
    AssetReader mAssetReader;
    GObjects mGameObjs;
    MYTRANS mTrans;
    OLDFONT* mpFont;

    //------------------------------------- Game
    SoccerBall*          m_pBall;
    SoccerTeam*          m_pRedTeam;
    SoccerTeam*          m_pBlueTeam;
    Goal*                m_pRedGoal;
    Goal*                m_pBlueGoal;
    //container for the boundary walls
    std::vector<Wall2D>  m_vecWalls;
//defines the dimensions of the playing area
    Region*              m_pPlayingArea;
 //the playing field is broken up into regions that the team can make use of to implement strategies.
    std::vector<Region*> m_Regions;
    bool m_bGameOn;
    bool m_bPaused;
    bool m_bGoalKeeperHasBall;
    bool m_bScoreFlag;
public:
    status onActivate();
    void onDeactivate();
    status onStep();

    void onStart();
    void onResume();
    void onPause();
    void onStop();
    void onDestroy();

    void onSaveInstanceState(void** pData, size_t* pSize);
    void onConfigurationChanged();
    void onLowMemory();

    void onCreateWindow();
    void onDestroyWindow();
    void onGainFocus();
    void onLostFocus();

};
#endif
