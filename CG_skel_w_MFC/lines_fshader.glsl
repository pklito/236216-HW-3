#version 150

uniform vec3 color;
out vec4 fColor;


void main() 
{ 
   fColor = vec4(color,1.0);
} 
