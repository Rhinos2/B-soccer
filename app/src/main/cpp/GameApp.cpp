#include "GameApp.h"
#include "Log.h"
#include "GLES.h"
#include "ConstantsToo.h"

#include <unistd.h>
#include <stdio.h>
#include "./game/TeamStates.h"


GameApp::GameApp(android_app* pApplication):

    mGraphicsManager(pApplication),
    mAssetReader(pApplication),
    mEventLoop(pApplication, *this, mInputHandler),
    m_Regions(NumRegionsHor*NumRegionsVer)
{
    mpFont = NULL;
    m_bGameOn = true;
    m_bGoalKeeperHasBall = false;
    m_bPaused = false;
    m_bScoreFlag = false;
}

void GameApp::run() {
    mEventLoop.run();
}

status GameApp::onStep() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static bool bCommence = true;
    if (bCommence)//start game
    {
         m_bGameOn = false;
        //get the teams ready for kickoff
        m_pBall->PlaceAtPosition(Vector2D(0.0f, 0.0f));
        m_pRedTeam->GetFSM()->ChangeState(PrepareForKickOff::Instance());
        m_pBlueTeam->GetFSM()->ChangeState(PrepareForKickOff::Instance());
        bCommence = false;
        return mGraphicsManager.SwapBuffers();;
    }
    if (!m_bPaused) {
        m_pBall->Update();
        m_pRedTeam->Update();
        m_pBlueTeam->Update();

    }

    DrawPitch();
    RenderTeams();
    DrawBall();

    if (m_pBlueGoal->Scored(m_pBall) || m_pRedGoal->Scored(m_pBall) || m_bScoreFlag){
        m_bGameOn = false;
        m_bScoreFlag = false;
        //reset ball's position
        m_pBall->PlaceAtPosition(Vector2D(0.0f, 0.0f));
        m_pRedTeam->GetFSM()->ChangeState(PrepareForKickOff::Instance());
        m_pBlueTeam->GetFSM()->ChangeState(PrepareForKickOff::Instance());
    }
    UpdateScore(m_pBlueGoal->NumGoalsScored(), m_pRedGoal->NumGoalsScored());

    return mGraphicsManager.SwapBuffers();
}
void GameApp::DrawPitch(){
    mTrans.SetMmatrixMode(PROJECTION_MAT);
    mTrans.SetIdentity();
    mTrans.SetPerspective(45.0f, (GLfloat) viewport_matrix[2], (GLfloat) viewport_matrix[3], 0.01f, 50.0f);
    mTrans.SetMmatrixMode(MODEL_VIEW_MAT);
//-----------------------------------------------------------------//
    mTrans.PushMatrix();
    MYPROGRAM* prg = mGameObjs.GetProgram(std::string("PitchProg"));
    glUseProgram(prg->GetPID());
    Uniforms(prg, PITCH_SH);

    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string( "Pitch.obj"))]->BindBuffers();
    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string( "Pitch.obj"))]->DrawMesh();
    mTrans.PopMatrix();

     glBindBuffer(GL_ARRAY_BUFFER, 0);
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   //------------------------------------------------//
    mTrans.PushMatrix();
    prg = mGameObjs.GetProgram(std::string("MarkingsProg"));
    glUseProgram(prg->GetPID());
    Uniforms(prg, ORTHO);

    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string( "Markings.obj"))]->BindBuffers();
    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string( "Markings.obj"))]->DrawMesh();
    mTrans.PopMatrix();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}
void GameApp::DrawBall(){
    mTrans.SetMmatrixMode(PROJECTION_MAT);
    mTrans.SetIdentity();
    mTrans.SetPerspective(45.0f, (GLfloat) viewport_matrix[2], (GLfloat) viewport_matrix[3], 0.01f, 50.0f);
    mTrans.SetMmatrixMode(MODEL_VIEW_MAT);
//-----------------------------------------------------------------// Ball drawing
    mTrans.PushMatrix();
    float h ;
    if(m_bGameOn) h = 0.5f;
    else h = 13.0f; //marks a scored ball
    mTrans.Translate(glm::vec3(m_pBall->Pos().x, h, m_pBall->Pos().y));

    MYPROGRAM* prg = mGameObjs.GetProgram(std::string("BallProg"));
    glUseProgram(prg->GetPID());
    Uniforms(prg, BALL_SH);

    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string("ball.obj"))]->BindBuffers();
    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string("ball.obj"))]->DrawMesh();
    mTrans.PopMatrix();///Rh
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GameApp::UpdateScore(int red, int blue){
    GLchar rSco[32], bSco[32];
    sprintf(rSco, "Red Bees: %i",red);
    sprintf(bSco, "Blue Bees: %i",blue);
    mTrans.SetMmatrixMode(PROJECTION_MAT);
    mTrans.SetIdentity();
    mTrans.SetOrthographic2D(0, ( float )viewport_matrix[ 2 ], 0, ( float )viewport_matrix[ 3 ]);
    //Red team score
    mTrans.PushMatrix();
    glUseProgram(mpFont->fontprg->GetPID());
    mTrans.Translate(glm::vec3(0.0f,( float )(viewport_matrix[ 3 ]*0.85), 0.0f));
    mTrans.Rotate(-90.0f, glm::vec3(0.0f, 0.0f, 1.0f) );
    glm::vec4 colorRed (1.0f, 0.0f, 0.0f, 1.0f );
    mpFont->FONT_print(0.0f, 5.0f, &rSco[0], mTrans.GetPRJmat(), colorRed);
    mTrans.PopMatrix();
    //Blue team score
    mTrans.PushMatrix();
    mTrans.Translate(glm::vec3(0.0f,( float )viewport_matrix[ 3 ]*0.45, 0.0f));
    mTrans.Rotate(-90.0f, glm::vec3(0.0f, 0.0f, 1.0f) );
    glm::vec4 colorBlue (0.0f, 0.0f, 1.0f, 1.0f );
    mpFont->FONT_print(0.0f, 5.0f, &bSco[0], mTrans.GetPRJmat(), colorBlue);
    mTrans.PopMatrix();

}

void GameApp::CreateRegions(float width, float height){
    //index into the vector
    int idx = m_Regions.size()-1;

    for (int row=0; row<NumRegionsVer; row++)
    {
        for (int col=0; col<NumRegionsHor; col++)
        {
            m_Regions[idx--] = new Region(m_pPlayingArea->Left()-col*width, m_pPlayingArea->Top()-row*height,
                                          m_pPlayingArea->Left()-(col+1)*width, m_pPlayingArea->Top()-(row+1)*height,
                                          idx);
        }
    }
}

