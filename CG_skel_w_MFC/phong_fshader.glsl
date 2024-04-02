
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

in vec3 vPos;

// 0 when time is not used.
uniform float time;

// Flag to determine whether to use wood texture or not
uniform bool useWoodTexture;

// Wood texture generation parameters
const int woodTextureWidth = 512;
const int woodTextureHeight = 512;
const float woodTextureScale = 5.0;

float mod289(float x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x * 34.0) + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float perlinNoise(vec3 P) {
    vec3 Pi0 = floor(P); // Integer part for indexing
    vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    vec3 Pf0 = fract(P); // Fractional part for interpolation
    vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    vec4 iy = vec4(Pi0.yy, Pi1.yy);
    vec4 iz0 = Pi0.zzzz;
    vec4 iz1 = Pi1.zzzz;

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);

    vec4 gx0 = ixy0 * (1.0 / 7.0);
    vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
    gx0 = fract(gx0);
    vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
    vec4 sz0 = step(gz0, vec4(0.0));
    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

    vec4 gx1 = ixy1 * (1.0 / 7.0);
    vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
    gx1 = fract(gx1);
    vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
    vec4 sz1 = step(gz1, vec4(0.0));
    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
    gy1 -= sz1 * (step(0.0, gy1) - 0.5);

    vec3 g000 = vec3(gx0.x, gy0.x, gz0.x);
    vec3 g100 = vec3(gx0.y, gy0.y, gz0.y);
    vec3 g010 = vec3(gx0.z, gy0.z, gz0.z);
    vec3 g110 = vec3(gx0.w, gy0.w, gz0.w);
    vec3 g001 = vec3(gx1.x, gy1.x, gz1.x);
    vec3 g101 = vec3(gx1.y, gy1.y, gz1.y);
    vec3 g011 = vec3(gx1.z, gy1.z, gz1.z);
    vec3 g111 = vec3(gx1.w, gy1.w, gz1.w);

    vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;

    float n000 = dot(g000, Pf0);
    float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
    float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
    float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
    float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
    float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
    float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
    float n111 = dot(g111, Pf1);

    vec3 fade_xyz = fade(Pf0);
    vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
    vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
    float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
    return 2.2 * n_xyz;
}

// Generate wood texture
vec3 generateWoodTexture(vec3 point) {
    // Adjust texture coordinates based on scale

    float color = perlinNoise(woodTextureScale * point);

    return vec3(color, color, color); // Use the noise value for all RGB components
}

vec3 getColorAnimation(vec3 diffuse_mat, float seed){
   // Define color increments
   float r_increment = abs(vPos.x/10);
   float g_increment = abs(vPos.y/10);
   float b_increment = abs(vPos.z/10);
   float colorModificationFactor = 0.0;
   if (time != 0.0) {
      // Calculate a factor based on time to gradually change the color
      colorModificationFactor = abs(sin(0.3*time * (1+seed)) * (10 - seed)); // Adjust the amplitude and frequency as needed
   }

   // Modify diffuse material based on the time factor
   vec3 diffuse_mat_modified = diffuse_mat;
   diffuse_mat_modified.r += r_increment * colorModificationFactor;
   diffuse_mat_modified.g += g_increment * colorModificationFactor;
   diffuse_mat_modified.b += b_increment * colorModificationFactor;

   // Clamp modified diffuse material to the range [0, 1]
   return clamp(diffuse_mat_modified, 0.0, 1.0);
}

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
   // If the flag to use wood texture is raised, apply wood texture
    if (useWoodTexture) {
        // Sample wood texture
        vec3 woodColor = generateWoodTexture(fPos.xyz);
        // Assign wood texture color to the fragment
        fColor = vec4(woodColor, 1.0);
    } else {
   //Decide if to take the materials from the vertices or from a uniform
   vec3 ambient_mat = fAmbient;
   vec3 diffuse_mat = fDiffuse;
   vec3 specular_mat = fSpecular;

   if(!isZero(uniform_material)){
      ambient_mat = uniform_material[0];
      diffuse_mat = uniform_material[1];
      specular_mat = uniform_material[2];
   }

   vec3 diffuse_mat_modified = getColorAnimation(diffuse_mat, 0);
   vec3 ambient_mat_modified = getColorAnimation(ambient_mat, 1.5);

   //calculate the color via light sources
   vec3 color = vec3(0,0,0);

   // point lights loop
   for(int i = 0; i < 10; ++i){
      vec3 dir = normalize(point_lights[i][1] - fPos.xyz);
      color += specular_calc(point_lights[i][0], dir, specular_mat);
      color += diffuse_calc(point_lights[i][0], dir, diffuse_mat_modified);
   }
   // directional lights
   for(int i = 0; i < 10; ++i){
      vec3 dir = directional_lights[i][1];
      color += specular_calc(directional_lights[i][0], dir, specular_mat);
      color += diffuse_calc(directional_lights[i][0], dir, diffuse_mat_modified);
   }


      //ambient lights
      color += ambient_light * ambient_mat_modified;
      fColor = vec4(color, 1);
   }
} 
