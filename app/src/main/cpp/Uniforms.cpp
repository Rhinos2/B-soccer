//
//
//
#include "GLMath.h"
#include "MyProgram.h"
#include "ConstantsToo.h"
#include "GameApp.h"


void GameApp::Uniforms(MYPROGRAM* prg, Shaders sdr){
    GLint uniform;
    if(sdr == ORTHO){
        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "MODELVIEWPROJECTIONMATRIX");
        glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(mTrans.GetPRJmat() * mTrans.GetMVmat()));

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "COLOR");
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniform, 1, (float *) &color);
    }

   else if(sdr == PITCH_SH){
        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "MODELVIEWPROJECTIONMATRIX");
        glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(mTrans.GetPRJmat() * mTrans.GetMVmat()));

    }
    else if(sdr == PLAYERS_RSH || sdr == PLAYERS_BSH) {
///////////////////////////////////////////////////////////////////////////////////////PERFRAG SHD
        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "MODELVIEWPROJECTIONMATRIX");
        glUniformMatrix4fv(uniform, 1, GL_FALSE,
                           glm::value_ptr(mTrans.GetPRJmat() * mTrans.GetMVmat()));

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "NORMALMATRIX");
        glUniformMatrix3fv(uniform, 1, GL_FALSE, glm::value_ptr(mTrans.GetNormalmat()));

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "MODELVIEWMATRIX");
        glUniformMatrix4fv(uniform, 1, GL_FALSE, glm::value_ptr(mTrans.GetMVmat() * mTrans.GetMODmat()));

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "LIGHTPOSITION");
        glm::vec4 Lpos4;
        if(sdr == PLAYERS_BSH)
            Lpos4 = mTrans.GetVmat() * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
        else
            Lpos4 = mTrans.GetVmat() * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 Lposit = glm::vec3(Lpos4);
        glUniform3fv(uniform, 1, (float *) &Lposit);

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "LIGHTINTENSITY");
        glm::vec3 lInts(0.5f, 0.7f, 0.7f);
        glUniform3fv(uniform, 1, (float *) &lInts);
        glUniform3fv(uniform, 1, (float *) &Lposit);

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "Material.Shininess");
        GLfloat ms(180.0f);
        glUniform1f(uniform, ms);

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "Material.Ka");
        glm::vec3 ka(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniform, 1, (float *) &ka);

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "Material.Kd");
        glm::vec3 kd(0.8f, 0.5f, 0.3f);
        glUniform3fv(uniform, 1, (float *) &kd);

        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "Material.Ks");
        glm::vec3 ks(0.9f, 0.9f, 0.9f);
        glUniform3fv(uniform, 1, (float *) &ks);                                    //PERFRAG SHDR
    }
    ////////////////////TOON
    else if(sdr == BALL_SH){
        uniform = prg->PROGRAM_get_uniform_location((GLchar *) "MODELVIEWPROJECTIONMATRIX");
        glUniformMatrix4fv(uniform, 1, GL_FALSE,
                           glm::value_ptr(mTrans.GetPRJmat() * mTrans.GetMVmat()));

    }
}