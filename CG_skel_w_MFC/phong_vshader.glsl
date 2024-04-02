#version 330

layout (location = 0) in  vec3 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec3 vTexture;
layout (location = 3) in  vec3 vMatAmb;
layout (location = 4) in  vec3 vMatDiff;
layout (location = 5) in  vec3 vMatSpec;
layout (location = 6) in vec3 aTangent;
layout (location = 7) in vec3 aBitangent; 
uniform mat4 world_transform;
uniform mat4 camera_transform;
uniform mat4 normal_transform;
out vec3 fNormal;
out vec2 fTexture;
out vec4 fPos;

out vec3 fDiffuse;
out vec3 fSpecular;
out vec3 fAmbient;

out vec3 vPos;

out mat3 TBN;

uniform float time;
void main()
{
    //world position for lights
    vec3 time_morph = 0.5 * (normalize(vPosition) * (sin(time) * sin(vPosition.x) * cos(vPosition.y) * sin(vPosition.z)));
    vec4 world_pos = world_transform * vec4(vPosition + time_morph ,1.0);
    fPos = world_pos;
    fNormal = normalize((normal_transform * vec4(vNormal,1.0)).xyz);
    fTexture = vTexture.xy;

    vPos = vPosition;
    
    fAmbient = vMatAmb;
    fDiffuse = vMatDiff;
    fSpecular = vMatSpec;
    //screen position
    gl_Position = camera_transform * (world_pos);

    //tangents
    if(length(aTangent) < 0.1){
        TBN = mat3(vec3(1,0,0),vec3(0,1,0),vec3(0,0,1));
    }
    else{
        vec3 T = normalize(vec3(world_transform * vec4(aTangent,   0.0)));
        vec3 B = normalize(vec3(world_transform * vec4(aBitangent, 0.0)));
        vec3 N = normalize(vec3(world_transform * vec4(vNormal,    0.0)));
        TBN = mat3(T, B, N);
    }
}