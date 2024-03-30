#version 330

layout (location = 0) in  vec3 vPosition;

uniform mat4 world_transform;

uniform mat4 camera_transform;
uniform mat4 normal_transform;

varying vec3 fNormal;

void main()
{
    // Include the normal_transform uniform in a calculation
    vec4 temp = normal_transform * vec4(vPosition, 1.0);

    // Pass the result through to the output
    fNormal = temp.xyz;
    //world position for lights
    vec4 world_pos = world_transform * vec4(vPosition,1.0);
    //screen position
    gl_Position = camera_transform * world_pos;
}