

varying mediump vec3 Position;
varying mediump vec3 Normal;


struct LightInfo {
   mediump vec4 position;
   mediump vec3 intensity;
};
uniform LightInfo Light;


uniform mediump vec3 Kd;            // Diffuse reflectivity
uniform mediump vec3 Ka;            // Ambient reflectivity

//const int levels = 3;
const lowp float scaleFactor = 0.222;



mediump vec3 toonShade( )
{
    mediump vec3 n = Normal;
    mediump vec3 s = normalize(Light.position.xyz - Position.xyz );
    mediump vec3 ambient = Ka;
    mediump float cosine = dot( s, n );
    mediump vec3 diffuse = Kd * ceil( cosine * 5.0 ) * scaleFactor;

    return Light.intensity * (ambient + diffuse);
}

void main() {
    gl_FragColor = vec4(toonShade(), 1.0);
}


