#version 150


in  vec3 vPosition;
in  vec3 vNormal;
in  vec3 vTexture;
varying vec3 fNormal;

void main()
{
    gl_Position = vec4(vPosition,1.0);
    fNormal = vTexture;
}