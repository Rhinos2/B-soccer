//
//
//

#ifndef SIM_CONSTANTS_H
#define SIM_CONSTANTS_H


#define VERTEX_SHADER_SIMP ( char * )"simply_grs.vs"
#define FRAGMENT_SHADER_SIMP ( char * )"simply_grs.fs"

#define VERTEX_SHADER_SIMP_CLR ( char * )"simply_clr.vs"
#define FRAGMENT_SHADER_SIMP_CLR ( char * )"simply_clr.fs"

#define VERTEX_SHADER_PER ( char * )"perfrag.vs"
#define FRAGMENT_SHADER_PER ( char * )"perfrag.fs"

#define VERTEX_SHADER_TOON ( char * )"simply_red.vs"
#define FRAGMENT_SHADER_TOON ( char * )"simply_red.fs"

#define DEBUG_SHADERS 1
#define FONTT (char* )"foo.ttf"

enum Shaders{ PITCH_SH = 1, PLAYERS_RSH = 2, PLAYERS_BSH =3, BALL_SH = 4, ORTHO};

const int NumRegionsHor = 3;
const int NumRegionsVer   = 6;

#endif
