#version 330

layout (location = 0) in  vec3 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec3 vTexture;
layout (location = 3) in  vec3 vMatAmb;
layout (location = 4) in  vec3 vMatDiff;
layout (location = 5) in  vec3 vMatSpec;
uniform mat4 world_transform;
uniform mat4 camera_transform;
uniform mat4 normal_transform;
varying vec3 fNormal;
varying vec2 fTexture;
varying vec4 fPos;

varying vec3 mat_diffuse;
varying vec3 mat_specular;
varying vec3 mat_ambient;
void main()
{
    //world position for lights
    vec4 world_pos = world_transform * vec4(vPosition,1.0);
    fPos = world_pos;
    fNormal = normalize((normal_transform * vec4(vNormal,1.0)).xyz);
    fTexture = vTexture.xy;

    mat_ambient = vMatAmb;
    mat_diffuse = vMatDiff;
    mat_specular = vec3(0.2,0.2,0.2);
    //screen position
    gl_Position = camera_transform * world_pos;
}