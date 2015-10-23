#version 130

uniform mat4 modelview;

uniform vec4 light1_diffuse_product, light1_ambient_product, light1_specular_product;
uniform float shininess;

in vec3 fE;
in vec3 fN;
in vec3 fL;

void
main()
{
    vec4 diffuse1, specular1;

    vec3 E = normalize( fE );
    vec3 N = normalize( ( modelview*vec4(fN, 0) ).xyz );
    vec3 L = normalize( fL );
    vec3 H = normalize( L + E );

    float Kd = max( dot( L, N ), 0 );
    diffuse1 = Kd * light1_diffuse_product;

    float Ks = pow( max( dot( N, H ), 0 ), shininess );
    specular1 = Ks * light1_specular_product;
    if ( dot( L, N ) < 0.0 ) specular1 = vec4( 0.0, 0.0, 0.0, 1.0 );

    gl_FragData[0] = vec4( ( diffuse1 + specular1 + light1_ambient_product ).xyz, 1 );
    gl_FragData[1] = vec4( light1_diffuse_product.xyz, 1 );
}
