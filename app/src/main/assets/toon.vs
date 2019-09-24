

attribute mediump vec3 POSITION;
attribute mediump vec3 NORMAL;

varying mediump vec3 Position;
varying mediump  vec3 Normal;



uniform mediump mat4 MODELVIEWMATRIX;
uniform mediump mat3 NORMALMATRIX;
uniform mediump mat4 MODELVIEWPROJECTIONMATRIX;




void main()
{
           Normal = normalize( NORMALMATRIX * NORMAL);
           Position = vec3( MODELVIEWMATRIX * vec4(POSITION,1.0) );

           gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(POSITION,1.0);
}


