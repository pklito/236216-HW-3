
#version 150

in vec2 texCoord;
in vec3 Color; // Received color attribute from vertex shader

varying vec3 fNormal;
out vec4 fColor;

uniform sampler2D texture;

void main() 
{
   float diffuse = max(.5 + dot(fNormal,vec3(0,0,1)),0);
   float neg_diffuse = max(.5 + dot(fNormal,vec3(0,0,-1)),0);
   fColor = texture(texture, texCoord) + vec4((diffuse * Color)+(neg_diffuse*vec3(1,0,0)), 1.0);
} 
