#version 150

uniform vec3 color;
out vec4 fColor;

uniform sampler2D texture;

void main() 
{ 
   // Include the texture uniform in a calculation
    vec4 texColor = texture(texture, vec2(0.0, 0.0));

    // Perform other operations, such as combining the texture color with the color uniform
    vec4 finalColor = vec4(color, 1.0);

    // Output the final color
    fColor = finalColor;
} 
