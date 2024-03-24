#version 150


in  vec3 vPosition;
in  vec3 vNormal;
in  vec3 vTexture;
uniform mat4 full_transform;
varying vec3 fNormal;

void main()
{
    gl_Position = full_transform * vec4(vPosition,1.0);
    fNormal = vNormal;
}