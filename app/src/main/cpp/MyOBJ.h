//
//
//

#ifndef _MYOBJ_H
#define _MYOBJ_H

#include <map>
#include <vector>
#include <string.h>



#include "glm/glm.hpp"
#include "glm/fwd.hpp"

#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/gtx/normal.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/projection.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/perpendicular.hpp"
#include "MyProgram.h"
//#include "Timing.h"
//#include "GraphEdge.h"

struct OBJ_MATERIAL
{
    GLchar					m_name[64];				// newmtl
    glm::vec4			    ambient;						// Ka
    glm::vec4				diffuse;						// Kd
    glm::vec4				specular;						// Ks
    glm::vec3				transmission_filter;			// Tf

    GLint					illumination_model;				// illum
    GLfloat					dissolve;						// d
    GLfloat					specular_exponent;				// Ns
    GLfloat					optical_density;				// Ni

    GLchar					map_ambient_file[ 64 ];		// map_Ka
    GLchar					map_diffuse_file[ 64 ];		// map_Kd
    GLchar					map_specular_file[ 64 ];		// map_Ks

    bool bAmbient;
    bool bDiffuse;
    bool bSpecular;

    OBJ_MATERIAL() {bAmbient = bDiffuse = bSpecular = false;}
} ;


struct VERTX
{
    glm::vec3   V;

    glm::vec3   Vn;
    glm::vec4   Tgt;
    GLuint      nIndex;
    VERTX()
    {
        V.x  = V.y = V.z = 0.0f;

        Vn.x    =  Vn.y  =  Vn.z = 0.0f;
        Tgt.x   = Tgt.y = Tgt.z = 0.0f;
        nIndex = 0;
    }
};

struct  IFS
{
    GLint NodeID;
    bool   bWalkable;
    int Reachability;
    GLuint PosInx[3];
    GLuint VtInx[3];
    GLuint VnInx[3];
    glm::vec3 triNormal;
    glm::vec3 centGrav;
 //   std::vector<NavGraphEdge> Edges;
    IFS(){
        PosInx[0] = PosInx[1] = PosInx[2] = 0;
        VtInx[0] = VtInx[1] = VtInx[2] = 0;
        NodeID = -1;
        bWalkable = false;
        centGrav.x = centGrav.y = centGrav.z = 0.0f;
  //      Edges.reserve(3);
        Reachability = 0;
    }
    inline IFS& operator --(int) {
        PosInx[0]--;PosInx[1]--; PosInx[2]--;
        VtInx[0]--;VtInx[1]--; VtInx[2]--;
        return *this;
    }
};
class GObjects;

class MESH
{
    friend class GObjects;
    GLchar                      o_name[64]; //o
    GLchar                      UseMtl[64]; //usemtl
   // glm::vec3               location;
    glm::vec3			        min;
    glm::vec3			        max;
    GLfloat			            radius;
    glm::vec3			        dimension;
   // GLfloat			        distance;
    GLint                       mode;

    std::vector<glm::vec3>      vecVPos;
    GLuint                      n_vecVPos;
    std::vector<glm::vec3>      vecVNorm;
    GLuint                      n_vecVNorm;
    std::vector<glm::vec2>      vecVTexCoords;

    std::vector<GLushort>        vecInxUV;
    std::vector<GLushort>        vecInxV;
    std::vector<GLushort>        vecInxWalk;
    std::vector<GLushort>        vecInxOptz;
    GLuint                       n_vecInx;
   // GLuint                       n_vecInxOptim;
    GLint                        array_buffer_size;
    GLint                        array_buffer_size_navig;
    std::vector<IFS>             Triangles;
    std::vector<IFS>            TriWalkable;

    std::map<GLuint,GLuint>     VPind_to_VUind;
    std::map<GLuint,GLuint>     VUinx_to_VTinx;
    GLuint                      stride;
    std::vector<GLfloat>        vecVBO;
    std::vector<GLfloat>        vecVBOnavig;
    GLuint                      vboIds[2];
    std::vector<GLint>          attribs;
    GLint                       attribPos;
    GLint                       attribNor;
    GLint                       attribTex;
public:
    glm::vec3                   location;
    GLuint                       n_vecInxOptim;
    GLint                       texCount;
    GLboolean 			useVt;// = false;

public:
    MESH(GLchar* name){
        strcpy(o_name, name);
        n_vecVPos = 0;  n_vecInx = 0; n_vecVNorm = 0;
        attribPos = attribNor = attribTex = - 10;
        mode = GL_TRIANGLES;
        n_vecInxOptim = 0;

    }
    ~MESH() {
    }
    GLint Mode() { return mode; }
//public methods
public:
    bool OptimizeMesh(GLuint vertex_cache_size, std::vector<GLushort> &vecIndeces);
    void UpdateBoundMesh();
    void BuildVBO();
    void BuildVOBnavig();
    void BindLoadBuffers(bool tex, bool nav);
    void BindBuffers();
    void BuildVBO(const GLfloat walkableSlopeAngle, const GLfloat walkableElevation);
    void SetMeshAttribs( MYPROGRAM *prg);
    void DrawMesh();
    void DrawNavArea();
    glm::vec3 GetMinDimention(){ return min; }
    glm::vec3 GetMaxDimention(){ return max; }
    glm::vec3 GetSize() { return dimension; }
    GLushort* GetV_InxBuffer();
    std::vector<GLushort> GetUV_InxBuffer();
    GLushort* GetOptimInxBuffer();

    std::map<GLuint,GLuint>& GetUV_toV_Map() { return  VUinx_to_VTinx; };
    std::vector<glm::vec3>& GetVPosBuffer();
    std::vector<glm::vec3>& GetVNormBuffer();
    std::vector<glm::vec2>& GetVTexBuffer();
    GLuint VPosBufferSize() { return n_vecVPos; }
    GLuint TexBufferSize() { return vecVTexCoords.size(); }
    GLuint NormBufferSize() { return  n_vecVNorm; }
    GLuint InxBufferSize();// { return  n_vecInx; }
    std::vector<IFS>& GetTriangles(bool walkable = false) { if(!walkable) return Triangles; else return TriWalkable;}

    void Barycentric(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p, float &u, float &v, float &w);

};
class GObjects{
    std::map<std::string, GLint>    LookUpMesh;
    GLint                           numMesh;
public:
    //TimingData*                     timingData;
    std::vector<MESH*>              GObjs;
    std::map<std::string, MYPROGRAM*>   mProgs;
    OBJ_MATERIAL                    Material;
    bool                            isMTL;
    GLchar                          MtlLib[64]; //mtllib

    GObjects() { isMTL = false; numMesh = 0;}
    ~GObjects();
    GLint LookupMeshNum(std::string mesh_name) { return LookUpMesh[mesh_name]; }
    void AddProgram(std::string name, MYPROGRAM* prg) { mProgs[name] = prg; }
    MYPROGRAM* GetProgram(std::string name) { return mProgs[name]; }
    std::map<std::string, MYPROGRAM*> GetProgram(){ return mProgs; }
    bool ReadOBJ(AssetReader& assetReader, GLchar* obj_name);

};

#endif
