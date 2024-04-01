#version 330

//vertex data
layout (location = 0) in  vec3 vPosition;
layout (location = 1) in  vec3 vNormal;
layout (location = 2) in  vec3 vTexture;
layout (location = 3) in  vec3 vMatAmb;
layout (location = 4) in  vec3 vMatDiff;
layout (location = 5) in  vec3 vMatSpec;
//transformations
uniform mat4 world_transform;
uniform mat4 camera_transform;
uniform mat4 normal_transform;
//lights
uniform mat2x3 point_lights[10];
uniform mat2x3 directional_lights[10];
uniform vec3 ambient_light;


uniform vec3 camera_position;

uniform mat3 uniform_material;

//assuming matrix elements are non negative, I want to know if the matrix is all zeros, or close to it
bool isZero(mat3 matrix){
   return dot(matrix*vec3(1,1,1), vec3(1,1,1)) < 0.01;
}

vec3 reflect_ray(vec3 vector, vec3 normal) {
    return vector - normal * 2 * dot(vector, normal);
}

vec3 specular_calc(vec3 light_color, vec3 light_direction, vec3 specular_mat, vec4 fPos, vec3 fNormal){
   //TODO get material from vshader

   vec3 view_direction = camera_position - fPos.xyz;
   float cos_phi = dot(reflect_ray(-light_direction,fNormal),view_direction);
   return specular_mat * light_color * pow(cos_phi, 3);  //TODO change 5 to uniform
}

vec3 diffuse_calc(vec3 light_color, vec3 direction, vec3 diffuse_mat, vec4 fPos, vec3 fNormal){
   return diffuse_mat * light_color * dot(direction, fNormal);
}
//outputs
out vec4 vColor;

void main()
{
    //world position for lights
    vec4 world_pos = world_transform * vec4(vPosition,1.0);
    vec4 fPos = world_pos;
    vec3 fNormal = normalize((normal_transform * vec4(vNormal,1.0)).xyz);
    vec2 fTexture = vTexture.xy;

    //Decide if to take the materials from the vertices or from a uniform
    vec3 ambient_mat = vMatAmb;
    vec3 diffuse_mat = vMatDiff;
    vec3 specular_mat = vMatSpec;

    if(!isZero(uniform_material)){
        ambient_mat = uniform_material[0];
        diffuse_mat = uniform_material[1];
        specular_mat = uniform_material[2];
    }

    //calculate the color via light sources
    vec3 color = vec3(0,0,0);
    // point lights loop
    for(int i = 0; i < 10; ++i){
        vec3 dir = normalize(point_lights[i][1]-fPos.xyz);
        color += specular_calc(point_lights[i][0], dir, specular_mat, fPos, fNormal);
        color += diffuse_calc(point_lights[i][0], dir, diffuse_mat, fPos, fNormal);
    }
    // directional lights
    for(int i = 0; i < 10; ++i){
        vec3 dir = directional_lights[i][1];
        color += specular_calc(directional_lights[i][0], dir, specular_mat, fPos, fNormal);
        color += diffuse_calc(directional_lights[i][0], dir, diffuse_mat, fPos, fNormal);
    }

    //ambient lights
    color += ambient_light * ambient_mat;
    //screen position
    gl_Position = camera_transform * world_pos;
    vColor = vec4(color,1);
}