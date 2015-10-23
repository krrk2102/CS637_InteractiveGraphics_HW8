#version 130

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 eyeposition;

uniform vec4 light1_pos;
uniform vec4 light1_diffuse_product, light1_ambient_product, light1_specular_product;
uniform float shininess;

in vec4 vPosition;
in vec4 vNormal;

out vec3 fE;
out vec3 fN;
out vec3 fL;

void
main()
{
    gl_Position = projection * modelview * vPosition;

    fN = vNormal.xyz;
    fE = -(modelview*vPosition).xyz;

    if ( light1_pos.w != 0.0 ) fL = light1_pos.xyz - (modelview*vPosition).xyz;
    else fL = light1_pos.xyz;
}
