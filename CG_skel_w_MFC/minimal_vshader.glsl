#version 330

layout (location = 0) in  vec3 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec3 vTexture;
uniform mat4 full_transform;
varying vec3 fNormal;

void main()
{
    gl_Position = full_transform * vec4(vPosition,1.0);
    fNormal = vNormal;
}