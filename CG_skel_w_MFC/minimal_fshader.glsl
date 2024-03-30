
#version 150

varying vec4 fPos;
varying vec3 fNormal;
out vec4 fColor;
uniform mat2x3 point_lights[10];
uniform mat2x3 directional_lights[10];
uniform vec3 ambient_light;
uniform vec3 camera_position;
varying vec3 mat_diffuse;
varying vec3 mat_ambient;
varying vec3 mat_specular;
vec3 reflect(vec3 vector, vec3 normal) {
    return vector - normal * 2 * dot(vector, normal);
}

vec3 specular_calc(vec3 light_color, vec3 light_direction){
   //TODO get material from vshader

   vec3 view_direction = camera_position - fPos.xyz;
   float cos_phi = dot(reflect(-light_direction,fNormal),view_direction);
   return mat_specular * light_color * pow(cos_phi, 3);  //TODO change 5 to uniform
}

vec3 diffuse_calc(vec3 light_color, vec3 direction){
   return mat_diffuse * light_color * dot(direction, fNormal);
}

void main() 
{ 

   vec3 color = vec3(0,0,0);
   // point lights loop
   for(int i = 0; i < 10; ++i){
      vec3 dir = normalize(point_lights[i][1]-fPos.xyz);
      color += specular_calc(point_lights[i][0], dir );
      color += diffuse_calc(point_lights[i][0], dir );
   }
   // directional lights
   for(int i = 0; i < 10; ++i){
      vec3 dir = directional_lights[i][1];
      color += specular_calc(directional_lights[i][0], dir);
      color += diffuse_calc(directional_lights[i][0], dir);
   }

   //ambient lights
   color += ambient_light * mat_ambient;
   fColor = vec4(color,1.0);
} 
