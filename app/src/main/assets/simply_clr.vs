
uniform mediump mat4 MODELVIEWPROJECTIONMATRIX;
uniform mediump vec3 COLOR;
attribute mediump vec3 POSITION;
attribute lowp vec3 NORMAL;


varying lowp vec3 color;

void main( void ) {
   color = COLOR;
   gl_Position = MODELVIEWPROJECTIONMATRIX * vec4( POSITION, 1.0 );
}