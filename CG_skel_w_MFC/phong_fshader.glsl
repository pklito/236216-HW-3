
#version 150

in vec4 fPos;
in vec3 fNormal;
out vec4 fColor;
uniform mat2x3 point_lights[10];
uniform mat2x3 directional_lights[10];
uniform vec3 ambient_light;
uniform vec3 camera_position;

uniform mat3 uniform_material;

in vec3 fDiffuse;
in vec3 fAmbient;
in vec3 fSpecular;
in vec2 fTexture;

// 0 when time is not used.
uniform float time;

//HSV, Taken from http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl. (https://stackoverflow.com/a/17897228)
// All components are in the range [0…1], including hue.
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
// All components are in the range [0…1], including hue.
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//assuming matrix elements are non negative, I want to know if the matrix is all zeros, or close to it
bool isZero(mat3 matrix){
   return dot(matrix*vec3(1,1,1), vec3(1,1,1)) < 0.01;
}

vec3 reflect_ray(vec3 vector, vec3 normal) {
    return vector - normal * 2 * dot(vector, normal);
}

vec3 specular_calc(vec3 light_color, vec3 light_direction, vec3 specular_mat){
   //TODO get material from vshader

   vec3 view_direction = normalize(camera_position - fPos.xyz);
   float cos_phi = max(dot(reflect_ray(-light_direction,fNormal),view_direction),0);
   return specular_mat * light_color * pow(cos_phi, 3);  //TODO change 5 to uniform
}

vec3 diffuse_calc(vec3 light_color, vec3 direction, vec3 diffuse_mat){
   return diffuse_mat * light_color * max(dot(direction, fNormal),0);
}

void main() 
{ 
   //Decide if to take the materials from the vertices or from a uniform
   vec3 ambient_mat = fAmbient;
   vec3 diffuse_mat = fDiffuse;
   vec3 specular_mat = fSpecular;

   if(!isZero(uniform_material)){
      ambient_mat = uniform_material[0];
      diffuse_mat = uniform_material[1];
      specular_mat = uniform_material[2];
   }

   //calculate the color via light sources
   vec3 color = vec3(0,0,0);
   // point lights loop
   for(int i = 0; i < 10; ++i){
      vec3 dir = normalize(point_lights[i][1] - fPos.xyz);
      color += specular_calc(point_lights[i][0], dir, specular_mat);
      color += diffuse_calc(point_lights[i][0], dir, diffuse_mat);
   }
   // directional lights
   for(int i = 0; i < 10; ++i){
      vec3 dir = directional_lights[i][1];
      color += specular_calc(directional_lights[i][0], dir, specular_mat);
      color += diffuse_calc(directional_lights[i][0], dir, diffuse_mat);
   }

   //ambient lights
   color += ambient_light * ambient_mat;

   fColor = vec4(color, 1);
} 
