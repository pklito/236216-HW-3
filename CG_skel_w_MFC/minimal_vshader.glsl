#version 330

layout (location = 0) in  vec3 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec3 vTexture;
uniform mat4 world_transform;
uniform mat4 camera_transform;
uniform mat4 normal_transform;
varying vec3 fNormal;
varying vec4 fPos;
void main()
{
    //world position for lights
    vec4 world_pos = world_transform * vec4(vPosition,1.0);
    fPos = world_pos;
    fNormal = normalize((normal_transform * vec4(vNormal,1.0)).xyz);

    //screen position
    gl_Position = camera_transform * world_pos;
}