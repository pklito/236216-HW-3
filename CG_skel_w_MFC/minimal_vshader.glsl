#version 330

layout (location = 0) in  vec3 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec2 vTexture;
layout(location = 3) in vec3 color; // Add color attribute

uniform mat4 world_transform;
uniform mat4 camera_transform;
uniform mat4 normal_transform;
out vec3 fNormal;

out vec2 texCoord;
out vec3 Color; // Pass color to fragment shader


void main()
{
    //world position for lights
    vec4 world_pos = world_transform * vec4(vPosition,1.0);
    fNormal = (normal_transform * vec4(vNormal,1.0)).xyz;

    //screen position
    gl_Position = camera_transform * world_pos;
    texCoord = vTexture;
    Color = color;
}