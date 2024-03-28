
#version 150

varying vec3 fNormal;
out vec4 fColor;
uniform mat2x3 point_lights[10];
uniform mat2x3 directional_lights[10];
uniform vec3 ambient_light;

void main() 
{ 
   //temp material
   vec3 diffuse_mat = vec3(0.3,0.3,0.3);
   vec3 specular_mat = vec3(0.3,0.3,0.3);
   vec3 ambient_mat = vec3(0.2,0.2,0.2);
   vec3 color = vec3(0,0,0);
   for(int i = 0; i < 10; ++i){
      color += diffuse_mat * point_lights[i][0] * max(0,dot(point_lights[i][1],fNormal));
   }
   color += ambient_light * ambient_mat;
   fColor = vec4(color,1.0);
} 
