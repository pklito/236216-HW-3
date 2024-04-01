
#version 150

varying vec4 fPos;
varying vec3 fNormal;
out vec4 fColor;
uniform mat2x3 point_lights[10];
uniform mat2x3 directional_lights[10];
uniform vec3 ambient_light;
uniform vec3 camera_position;

uniform mat3 uniform_material;

varying vec3 fDiffuse;
varying vec3 fAmbient;
varying vec3 fSpecular;
varying vec2 fTexture;
uniform sampler2D ourTexture;

//assuming matrix elements are non negative, I want to know if the matrix is all zeros, or close to it
bool isZero(mat3 matrix){
   return dot(matrix*vec3(1,1,1), vec3(1,1,1)) < 0.01;
}

vec3 reflect(vec3 vector, vec3 normal) {
    return vector - normal * 2 * dot(vector, normal);
}

vec3 specular_calc(vec3 light_color, vec3 light_direction, vec3 specular_mat){
   //TODO get material from vshader

   vec3 view_direction = camera_position - fPos.xyz;
   float cos_phi = dot(reflect(-light_direction,fNormal),view_direction);
   return mat_specular * light_color * pow(cos_phi, 3);  //TODO change 5 to uniform
}

vec3 diffuse_calc(vec3 light_color, vec3 direction, vec3 diffuse_mat){
   return mat_diffuse * light_color * dot(direction, fNormal);
}

void main() 
{ 
   //Decide if to take the materials from the vertices or from a uniform
   vec3 ambient_mat = fAmbient;
   vec3 diffuse_mat = texture(ourTexture, fTexture).xyz;
   vec3 specular_mat = fSpecular;

   if(!isZero(uniform_material)){
      ambient_mat = uniform_material[0];
      specular_mat = uniform_material[2];
   }

   //calculate the color via light sources
   vec3 color = vec3(0,0,0);
   // point lights loop
   for(int i = 0; i < 10; ++i){
      vec3 dir = normalize(point_lights[i][1]-fPos.xyz);
      color += specular_calc(point_lights[i][0], dir, specular_mat);
      color += diffuse_calc(point_lights[i][0], dir, diffuse_mat);
   }
   // directional lights
   for(int i = 0; i < 10; ++i){
      vec3 dir = directional_lights[i][1];
      color += specular_calc(directional_lights[i][0], dir, specular_mat);
      color += diffuse_calc(directional_lights[i][0], dir diffuse_mat);
   }

   //ambient lights
   color += ambient_light * ambient_mat;

   fColor = vec4(color, 1);
} 
