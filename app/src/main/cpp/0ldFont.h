//
//
//

#ifndef DUMMY_0LDFONT_H
#define DUMMY_0LDFONT_H


#include <stdio.h>
#include "ttf/stb_truetype.h"
#include "MyProgram.h"
#include "glm/detail/type_mat.hpp"
#include "glm/vec4.hpp"

struct OLDFONT
{
    FILE*  stream;
    GLchar			name[ 64 ];
    stbtt_bakedchar *character_data;
    GLfloat			font_size;
    GLint				texture_width;
    GLint				texture_height;
    GLint				first_character;
    GLint				count_character;
    MYPROGRAM*		fontprg;
    GLuint	tid;
    GLuint buf;
    GLubyte*            ArrBuffer;
    OLDFONT(GLchar *nm) {  strcpy(name, nm); fontprg = NULL; }
    ~OLDFONT(){}
    GLshort FONT_init(AssetReader& assetReader);
    void FONT_free();
    GLshort FONT_load( AssetReader& assetReader, GLchar *filename, GLfloat font_size, GLuint texture_width, GLuint texture_height,
                       GLint first_character, GLint count_character );
    void FONT_print( GLfloat x, GLfloat y, const GLchar *text, glm::mat4& MVP, glm::vec4& color );
    GLfloat FONT_length(  GLchar *text );

} ;


#endif //DUMMY_0LDFONT_H
