

varying mediump vec3 Position;
varying mediump vec3 Normal;

uniform mediump vec4 LIGHTPOSITION;
uniform mediump vec3 LIGHTINTENSITY;


struct MaterialInfo {
  mediump vec3 Ka;            // Ambient reflectivity
  mediump vec3 Kd;            // Diffuse reflectivity
  mediump vec3 Ks;            // Specular reflectivity
  mediump float Shininess;    // Specular shininess factor
};
uniform MaterialInfo Material;


mediump vec3 halfway( )
{
    mediump vec3 s = normalize( vec3(LIGHTPOSITION) - Position );
    mediump vec3 v = normalize(vec3(-Position));
    mediump vec3 h = normalize( v + s );

    return
        LIGHTINTENSITY * (Material.Ka + Material.Kd * max( dot(s, Normal), 0.0 ) + Material.Ks * pow( max( dot(h, Normal), 0.0 ), Material.Shininess ) );
}

void main() {
    gl_FragColor = vec4(halfway(), 1.0);
}
