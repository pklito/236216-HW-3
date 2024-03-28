
#version 150

varying vec4 fPos;
varying vec3 fNormal;
out vec4 fColor;
uniform mat2x3 point_lights[10];
uniform mat2x3 directional_lights[10];
uniform vec3 ambient_light;

vec3 diffuse_calc(vec3 light_color, vec3 direction){
   vec3 diffuse_mat = vec3(1,0.3,0.3);
   return diffuse_mat * light_color * dot(direction, fNormal);
}

void main() 
{ 
   //temp material
   vec3 diffuse_mat = vec3(1,0.3,0.3);
   vec3 specular_mat = vec3(0.3,0.3,0.3);
   vec3 ambient_mat = vec3(0.2,0.2,0.2);

   vec3 color = vec3(0,0,0);
   // point lights loop
   for(int i = 0; i < 10; ++i){
      vec3 dir = normalize(point_lights[i][1]-fPos.xyz);
      color += diffuse_calc(point_lights[i][0], dir );
   }
   // directional lights
   for(int i = 0; i < 10; ++i){
      color += diffuse_calc(point_lights[i][0], point_lights[i][1]);
   }

   //ambient lights
   color += ambient_light * ambient_mat;
   fColor = vec4(color,1.0);
} 
