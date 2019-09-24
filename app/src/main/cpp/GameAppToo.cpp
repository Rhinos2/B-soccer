//
//
//

#include "GameApp.h"
#include "Log.h"
#include "GLES.h"
#include "ConstantsToo.h"

#include <unistd.h>
#include <stdio.h>
#include "./game/PlayerBase.h"
#include "./game/Goalkeeper.h"
#include "./game/FieldPlayer.h"
#include "./game/SupportSpotCalculator.h"

status GameApp::onActivate() {

    if (mGraphicsManager.start() != STATUS_OK) return STATUS_KO;

    GLES::GLES_Init();

    glViewport(0, 0, mGraphicsManager.getRenderWidth(), mGraphicsManager.getRenderHeight());
    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
    ////////////////////////////////////////////////////////////////////////view setting
    glm::vec3 eye(0.0f, 15.2f, -0.001f);
    glm::vec3 cent(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    mTrans.SetMmatrixMode(MODEL_VIEW_MAT);
    mTrans.SetIdentity();
    mTrans.SetLookAtOld(eye, cent, up);
    // Initializes game objects.
////////////////////////////////////////////////////////////////////////////// font
    mpFont = new OLDFONT(FONTT);
    if (0 != mpFont->FONT_init(mAssetReader))
        exit(1);
    int fontsize = mGraphicsManager.getRenderWidth()/11;
    if (0 != mpFont->FONT_load(mAssetReader, mpFont->name, (GLfloat)fontsize, 512, 512, 32, 96))
        exit(1);
    /////////////////////////////////////////////////////////////////////////shaders
    if(0 != CreateShaderProgs())
        exit(1);
////////////////////////////////////////////////////////////////////////////// objs loading
    if (!mGameObjs.ReadOBJ(mAssetReader, (GLchar *) "Markings.obj"))
        exit(1);
    GLint last_mesh = mGameObjs.LookupMeshNum(std::string( "Markings.obj"));
    mGameObjs.GObjs[last_mesh]->UpdateBoundMesh();
    mGameObjs.GObjs[last_mesh]->BuildVBO();
    mGameObjs.GObjs[last_mesh]->BindLoadBuffers(false, false);
    mGameObjs.GObjs[last_mesh]->SetMeshAttribs(mGameObjs.GetProgram(std::string("MarkingsProg")));
    //define the playing area
    m_pPlayingArea = new Region( mGameObjs.GObjs[last_mesh]->GetMaxDimention().x, mGameObjs.GObjs[last_mesh]->GetMaxDimention().z,
                                 mGameObjs.GObjs[last_mesh]->GetMinDimention().x, mGameObjs.GObjs[last_mesh]->GetMinDimention().z);
    if (!mGameObjs.ReadOBJ(mAssetReader, (GLchar *) "Pitch.obj"))
        exit(1);
    last_mesh = mGameObjs.LookupMeshNum(std::string( "Pitch.obj"));
    mGameObjs.GObjs[last_mesh]->UpdateBoundMesh();
    mGameObjs.GObjs[last_mesh]->BuildVBO();
    mGameObjs.GObjs[last_mesh]->BindLoadBuffers(false, false);
    mGameObjs.GObjs[last_mesh]->SetMeshAttribs(mGameObjs.GetProgram(std::string("PitchProg")));

    //--------------------------------------------------------------------//
    if (!mGameObjs.ReadOBJ(mAssetReader, (GLchar *) "player3.obj"))
        exit(1);
    last_mesh = mGameObjs.LookupMeshNum(std::string( "player3.obj"));
    mGameObjs.GObjs[last_mesh]->UpdateBoundMesh();
    mGameObjs.GObjs[last_mesh]->BuildVBO();
    mGameObjs.GObjs[last_mesh]->BindLoadBuffers(false,false);
    mGameObjs.GObjs[last_mesh]->SetMeshAttribs(mGameObjs.GetProgram(std::string("PlayersProg")));
    glm::vec3 PlayerSize = mGameObjs.GObjs[last_mesh]->GetSize();
//-------------------------------------------------------------------//
    if (!mGameObjs.ReadOBJ(mAssetReader, (GLchar *) "ball.obj"))
        exit(1);
    last_mesh = mGameObjs.LookupMeshNum(std::string( "ball.obj"));
    mGameObjs.GObjs[last_mesh]->UpdateBoundMesh();
    mGameObjs.GObjs[last_mesh]->BuildVBO();
    mGameObjs.GObjs[last_mesh]->BindLoadBuffers(false,false);
    mGameObjs.GObjs[last_mesh]->SetMeshAttribs(mGameObjs.GetProgram(std::string("BallProg")));
    glm::vec3 ballSize = mGameObjs.GObjs[last_mesh]->GetSize();
//----------------------------------------------------------------------//
    CreateRegions(m_pPlayingArea->Width()/NumRegionsHor, m_pPlayingArea->Height()/NumRegionsVer);

    m_pRedGoal  = new Goal(Vector2D( 1.0f, m_pPlayingArea->Top()), Vector2D(-1.0f, m_pPlayingArea->Top()),
                           Vector2D(0.0f,-1.0f));
    m_pBlueGoal = new Goal( Vector2D( 1.0f, m_pPlayingArea->Bottom()), Vector2D(-1.0f, m_pPlayingArea->Bottom()),
                            Vector2D(0.0f,1.0f));
    //create the walls
    Vector2D TopLeft(m_pPlayingArea->Left(), m_pPlayingArea->Top());
    Vector2D TopRight(m_pPlayingArea->Right(), m_pPlayingArea->Top());
    Vector2D BottomRight(m_pPlayingArea->Right(), m_pPlayingArea->Bottom());
    Vector2D BottomLeft(m_pPlayingArea->Left(), m_pPlayingArea->Bottom());

    Vector2D postMargin(0.1f, 0.0f);
    m_vecWalls.push_back(Wall2D(BottomLeft, m_pBlueGoal->LeftPost()-postMargin, Vector2D(0.0f, 1.0f)));
    m_vecWalls.push_back(Wall2D(m_pBlueGoal->RightPost()+postMargin, BottomRight, Vector2D(0.0f, 1.0f)));
    m_vecWalls.push_back(Wall2D(BottomRight, TopRight, Vector2D(1.0f, 0.0f)));
    m_vecWalls.push_back(Wall2D(TopRight, m_pRedGoal->RightPost()+postMargin, Vector2D(0.0f, -1.0f)));
    m_vecWalls.push_back(Wall2D(m_pRedGoal->LeftPost()-postMargin, TopLeft, Vector2D(0.0f, -1.0f)));
    m_vecWalls.push_back(Wall2D(TopLeft, BottomLeft, Vector2D(-1.0f, 0.0f)));

    // the soccer ball
    m_pBall = new SoccerBall(Vector2D(0.0f, 0.0f), ballSize.x, 1.5f,/*BallMass */m_vecWalls);
    // the teams
    m_pRedTeam  = new SoccerTeam(m_pRedGoal, m_pBlueGoal, this, SoccerTeam::red);
    m_pBlueTeam = new SoccerTeam(m_pBlueGoal, m_pRedGoal, this, SoccerTeam::blue);
    m_pRedTeam->SetOpponents(m_pBlueTeam);
    m_pBlueTeam->SetOpponents(m_pRedTeam);

    return STATUS_OK;
}

void GameApp::onDeactivate() {
    Log::info("Deactivating GameApp");
    mGraphicsManager.stop();
}
GLshort GameApp::CreateShaderProgs(){
    MYPROGRAM* prg = new MYPROGRAM((GLchar *) "MarkingsProg");
    if (0 != prg->PROGRAM_create(mAssetReader, VERTEX_SHADER_SIMP_CLR, FRAGMENT_SHADER_SIMP_CLR, DEBUG_SHADERS))
        return -1;
    if (0 != prg->PROGRAM_link(DEBUG_SHADERS))
        return -1;
    mGameObjs.AddProgram(std::string("MarkingsProg"), prg);

    prg = new MYPROGRAM((GLchar *) "PitchProg");
    if (0 != prg->PROGRAM_create(mAssetReader, VERTEX_SHADER_SIMP, FRAGMENT_SHADER_SIMP, DEBUG_SHADERS))
        return -1;
    if (0 != prg->PROGRAM_link(DEBUG_SHADERS))
        return -1;
    mGameObjs.AddProgram(std::string("PitchProg"), prg);

    prg = new MYPROGRAM((GLchar *) "BallProg");
    if (0 != prg->PROGRAM_create(mAssetReader, VERTEX_SHADER_TOON, FRAGMENT_SHADER_TOON, DEBUG_SHADERS))
        return -1;
    if (0 != prg->PROGRAM_link(DEBUG_SHADERS))
        return -1;
    mGameObjs.AddProgram(std::string("BallProg"), prg);

    prg = new MYPROGRAM((GLchar *) "PlayersProg");
    if (0 != prg->PROGRAM_create(mAssetReader, VERTEX_SHADER_PER, FRAGMENT_SHADER_PER, DEBUG_SHADERS))
        return -1;
    if (0 != prg->PROGRAM_link(DEBUG_SHADERS))
        return -1;
    mGameObjs.AddProgram(std::string("PlayersProg"), prg);
    return 0;
}

void GameApp::RenderTeams() {
    mTrans.SetMmatrixMode(PROJECTION_MAT);
    mTrans.SetIdentity();
    mTrans.SetPerspective(45.0f, (GLfloat) viewport_matrix[2], (GLfloat) viewport_matrix[3], 0.01f, 50.0f);
    mTrans.SetMmatrixMode(MODEL_VIEW_MAT);

    mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string("player3.obj"))]->BindBuffers();
//-------------------------------------------------------------------------------------------keepers
    std::vector<PlayerBase*>::const_iterator pos;
    for( pos = m_pRedTeam->Members().begin(); pos != m_pRedTeam->Members().end(); ++pos) {
        mTrans.PushMatrix();
        mTrans.Translate(glm::vec3((*pos)->Pos().x, 0.5f, (*pos)->Pos().y));
        float angDegree, angEven;
        if ((*pos)->GetHomeRegionID() == 16) // red goalkeeper
            angDegree = glm::degrees(glm::atan(((GoalKeeper *) (*pos))->LookAt().x,
                                               ((GoalKeeper *) (*pos))->LookAt().y));
        else{
            if(Vec2DLengthSq((*pos)->Velocity()) < 0.0001f)
                angDegree = glm::degrees(glm::atan(((FieldPlayer *) (*pos))->LookAt().x, ((FieldPlayer *) (*pos))->LookAt().y));
            else
                angDegree = glm::degrees(glm::atan(((*pos))->Heading().x, ((*pos))->Heading().y));
        }
        angEven = glm::roundEven(angDegree);
        mTrans.Rotate(angEven - 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        glUseProgram((mGameObjs.GetProgram(std::string("PlayersProg"))->GetPID()));
        Uniforms(mGameObjs.GetProgram(std::string("PlayersProg")),PLAYERS_RSH);
        mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string("player3.obj"))]->DrawMesh();
        mTrans.PopMatrix();///Rh
    }
    for( pos = m_pBlueTeam->Members().begin(); pos != m_pBlueTeam->Members().end(); ++pos) {
        mTrans.PushMatrix();
        mTrans.Translate(glm::vec3((*pos)->Pos().x, 0.5f, (*pos)->Pos().y));
        float angDegree, angEven;
        if((*pos)->GetHomeRegionID() == 1) {// blue goalkeeper
            angDegree = glm::degrees(glm::atan(((GoalKeeper *) (*pos))->LookAt().x, ((GoalKeeper *) (*pos))->LookAt().y));
        }
        else{
            if(Vec2DLengthSq((*pos)->Velocity()) < 0.0001f)
                angDegree = glm::degrees(glm::atan(((FieldPlayer *) (*pos))->LookAt().x, ((FieldPlayer *) (*pos))->LookAt().y));
            else
                angDegree = glm::degrees(glm::atan(((*pos))->Heading().x, ((*pos))->Heading().y));
        }

        angEven = glm::roundEven(angDegree);
        mTrans.Rotate(angEven - 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        glUseProgram((mGameObjs.GetProgram(std::string("PlayersProg"))->GetPID()));
        Uniforms(mGameObjs.GetProgram(std::string("PlayersProg")),PLAYERS_BSH);
        mGameObjs.GObjs[mGameObjs.LookupMeshNum(std::string("player3.obj"))]->DrawMesh();
        mTrans.PopMatrix();///Rh
    }
    //---------------------------------------------------------------------------keepers end
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GameApp::onStart() {}

void GameApp::onResume() {}

void GameApp::onPause() {}

void GameApp::onStop() {}

void GameApp::onDestroy() {

    mpFont->FONT_free();
    delete m_pPlayingArea;
    delete m_pRedGoal;
    delete m_pBlueGoal;
    delete m_pBall;
    std::vector<PlayerBase*>::const_iterator pos;
    for( pos = m_pRedTeam->Members().begin(); pos != m_pRedTeam->Members().end(); ++pos)
        delete *pos;
    for( pos = m_pBlueTeam->Members().begin(); pos != m_pBlueTeam->Members().end(); ++pos)
        delete *pos;
    for (unsigned int i=0; i<m_Regions.size(); ++i)
        delete m_Regions[i];
}

void GameApp::onSaveInstanceState(void** pData, size_t* pSize) {}
void GameApp::onConfigurationChanged() {}
void GameApp::onLowMemory() {}
void GameApp::onCreateWindow() {}
void GameApp::onDestroyWindow() {}
void GameApp::onGainFocus() {}
void GameApp::onLostFocus() {}
