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
out vec3 fNormal;
out vec2 fTexture;
out vec4 fPos;

out vec3 fDiffuse;
out vec3 fSpecular;
out vec3 fAmbient;

uniform float time;
void main()
{
    //world position for lights
    vec3 time_morph = 0.5 * (normalize(vPosition) * (sin(time) * sin(vPosition.x) * cos(vPosition.y) * sin(vPosition.z)));
    vec4 world_pos = world_transform * vec4(vPosition + time_morph ,1.0);
    fPos = world_pos;
    fNormal = normalize((normal_transform * vec4(vNormal,1.0)).xyz);
    fTexture = vTexture.xy;

    fAmbient = vMatAmb;
    fDiffuse = vMatDiff;
    fSpecular = vMatSpec;
    //screen position
    gl_Position = camera_transform * (world_pos);
}