#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include <cmath>
//#include "imgui.h"
#include "MeshModel.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define Z_INDEX(width,x,y) (x+y*width)
#define SUPER_INDEX(width,x,y) (x+y*width)
#define LINE_TOO_LARGE 30
#define CLIP_TO_SCREEN(x,y,z) (vec3(RANGE((x),-aspect_ratio, aspect_ratio, 0, m_width), RANGE((y),-1,1,0,m_height), (z)))
Renderer::Renderer() :m_width(512), m_height(512), aspect_ratio(1), curr_color(0), shading_method(FLAT), ambient_light(AmbientLight(0,vec3(0,0,0))), m_supersampledBuffer(NULL)
{
	InitOpenGLRendering();
	supersample_factor = 2;
	draw_fog = false;
	anti_aliasing = false;
	CreateBuffers(512,512);
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height), curr_color(0), shading_method(FLAT), ambient_light(AmbientLight(0,vec3(0,0,0))), m_supersampledBuffer(NULL)
{
	InitOpenGLRendering();
	supersample_factor = 2;
	draw_fog = false;
	anti_aliasing = false;
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
	ReleaseBuffers(); // Ensure that m_outBuffer and m_zbuffer are deleted
}

void BubbleSort(std::vector<int>& intersections) {
	int n = intersections.size();
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			if (intersections[j] > intersections[j + 1]) {
				// Swap if the current element is greater than the next element
				std::swap(intersections[j], intersections[j + 1]);
			}
		}
	}
}



void Renderer::CreateBuffers(int width, int height)
{
	//increase anti_aliasing
	int factor = anti_aliasing ? supersample_factor : 1;
	m_downsize_height = height;
	m_downsize_width = width;
	m_width=width * factor;
	m_height=height * factor;
	std::cout << "- Creating buffers with sizes " << width*factor << "," << height*factor << std::endl;
	aspect_ratio=(float)(m_width)/(float)(m_height);	
	CreateOpenGLBuffer(); //Do not remove this line.
	//CreateSupersampledBuffer();
	m_outBuffer = new float[3*m_width*m_height];
	m_zbuffer = new float[m_width * m_height];

	if(anti_aliasing){
		m_downsizeBuffer = new float[3*m_downsize_height*m_downsize_width];
	}
	else{
		m_downsizeBuffer = nullptr;
	}
}

void Renderer::ReleaseBuffers() {
	
	std::cout << "- Releasing buffers " << m_outBuffer << "," << m_downsizeBuffer << std::endl;
	delete[] m_outBuffer;
	delete[] m_zbuffer;
	delete[] m_supersampledBuffer;
	
	if(m_downsizeBuffer != nullptr){
		delete[] m_downsizeBuffer;
	}

	m_outBuffer = nullptr;
	m_zbuffer = nullptr;
	m_supersampledBuffer = nullptr;
	m_downsizeBuffer = nullptr;
}

void Renderer::CreateSupersampledBuffer()
{
	supersampled_width = m_width * supersample_factor;
	supersampled_height = m_height * supersample_factor;

	// Create a buffer for supersampling
	try{	
		m_supersampledBuffer = new vec3[supersampled_width*supersampled_height];
	}
	catch (exception e){
		std::cout << e.what() << std::endl;
		throw e;
	}
	//m_supersampledDepth.resize(supersampled_width, std::vector<float>(supersampled_height, 0.0f));
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,0)]=1;	m_outBuffer[INDEX(m_width,256,i,1)]=0;	m_outBuffer[INDEX(m_width,256,i,2)]=0;

	}
	//horizontal line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,0)]=1;	m_outBuffer[INDEX(m_width,i,256,1)]=0;	m_outBuffer[INDEX(m_width,i,256,2)]=1;

	}
}


void Renderer::ResizeBuffers(int new_width, int new_height) {
	int factor = anti_aliasing ? supersample_factor : 1;
	if (factor*new_width != m_width || factor*new_height != m_height) {
		ReleaseBuffers();
		CreateBuffers(new_width, new_height);
	}
}

void Renderer::ClearBuffer(){
	std::fill(m_outBuffer,m_outBuffer+(m_width*m_height*3),0);
	std::fill(m_zbuffer, m_zbuffer + (m_width * m_height), far_z);
	if(m_downsizeBuffer != nullptr){
		std::fill(m_downsizeBuffer, m_downsizeBuffer + (3*m_downsize_height*m_downsize_width), 0);
	}
}

void Renderer::FillBuffer(vec3 color)
{

	// Fill the buffer with the background color
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			DrawPixel(x,y,far_z,color);
		}
	}
}

void Renderer::setCameraPos(vec3 camera_pos)
{
	camera_position = camera_pos;
}

void Renderer::FillEdges(float percent, vec3 color) {
	if(percent <= 0){
		return;
	}
	if(percent >= 0.5){
		percent = 0.5;
	}
	for(int i = 0; i <= (int)(m_height*percent); i++){
		for(int j = 0; j < m_width; j++){
			DrawPixel(j,i,0,color);
			DrawPixel(j,m_height - i- 1,0,color);
		}
	}
	for(int j = 0; j < (int)(m_width*percent); j++){
		for(int i = (int)(m_height*percent) - 1; i < m_height - (int)(m_height*percent) - 1; i++){
			DrawPixel(j,i,0,color);
			DrawPixel(m_width - j - 1,i,0,color);
		}
	}
}


// Function to check for differences in color values
void Renderer::CheckColorDifferences(vec3* supersampledBuffer, const float* finalBuffer, int width, int height) {
	float epsilon = 1e-5;  // Adjust this threshold based on your expected color differences

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			vec3 accumulatedColor(0.0f, 0.0f, 0.0f);

			int validSamples = 0;
			// Accumulate colors from the supersampled buffer
			for (int i = 0; i < supersample_factor; i++)
			{
				for (int j = 0; j < supersample_factor; j++)
				{
					int sx = x * supersample_factor + i;
					int sy = y * supersample_factor + j;

					if (sx < 0 || sx >= supersampled_width || sy < 0 || sy >= supersampled_height)
					{
						// Skip this sample if it's outside the valid range
						std::cout << "SOMETHING IS WRONG" << std::endl;
						continue;
					}

					accumulatedColor += m_supersampledBuffer[SUPER_INDEX(supersampled_width,sx,sy)];
					validSamples++;	
				}
			}

			if (validSamples > 0)
			{
				accumulatedColor /= validSamples;
			}
			
			vec3 finalColor = vec3(
				finalBuffer[INDEX(width, x, y, 0)],
				finalBuffer[INDEX(width, x, y, 1)],
				finalBuffer[INDEX(width, x, y, 2)]
			);

			if (std::abs(accumulatedColor.x - finalColor.x) > epsilon ||
				std::abs(accumulatedColor.y - finalColor.y) > epsilon ||
				std::abs(accumulatedColor.z - finalColor.z) > epsilon) {
				// Print information about the discrepancy
				std::cout << "Color difference at (" << x << ", " << y << "): ";
				std::cout << "Supersampled: " << accumulatedColor << ", Final: " << finalColor << std::endl;

				m_outBuffer[INDEX(m_width, x, y, 0)] = accumulatedColor.x;	m_outBuffer[INDEX(m_width, x, y, 1)] = accumulatedColor.y;	m_outBuffer[INDEX(m_width, x, y, 2)] = accumulatedColor.z;
			}
		}
	}
}

/*
 This function gets two pixels on screen and draws the line between them (rasterization)
 vert1 + vert2 = ends of the edge
 normal = direction of normal.
*/
void Renderer::DrawLine(vec3 vert1, vec3 vert2, vec3 color)
{
	//flip the axis so that slope is -1 <= m <= 1
	bool flipped = false;
	if (abs(vert1.y - vert2.y) > abs(vert1.x - vert2.x)) {
		std::swap(vert1.x, vert1.y);
		std::swap(vert2.x, vert2.y);
		flipped = true;
	}

	// Swap the order so that vert1 is left of vert2
	if (vert1.x > vert2.x) {
		std::swap(vert1, vert2);
	}

	// Line drawing
	int x = vert1.x;
	int y = vert1.y;
	float z = vert1.z;

	int dx = vert2.x - vert1.x;
	int dy = abs(vert2.y - vert1.y);
	float dz = vert2.z - vert1.z;

	int slope_direction = (vert2.y >= vert1.y) ? 1 : -1;

	// Determine whether to increment or decrement y
	int y_increment = slope_direction;

	// Decision parameter
	int d = 2 * dy - dx;

	for (int i = 0; i <= dx; i++)
	{
		// Light the pixel
		if (flipped) {
			DrawPixelSafe(y, x, z, color);
		}
		else {
			DrawPixelSafe(x, y, z, color);
		}

		// Update the position
		x++;

		// Update the decision parameter
		if (d < 0) {
			d += 2 * dy;
		}
		else {
			y += y_increment;
			d += 2 * (dy - dx);
		}

		// Update z using linear interpolation
		z += dz / dx;
	}
}

void Renderer::DrawPixel(int x, int y, float z, vec3 color){
	if (z < m_zbuffer[Z_INDEX(m_width, x, y)]) {
		m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;	m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;	m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
		m_zbuffer[Z_INDEX(m_width, x, y)] = z;
	}		
}

void Renderer::DrawPixelSafe(int x, int y, float z, vec3 color){
	if(x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;
	DrawPixel(x,y,z,color);
}

/**
 * This function takes a world space object, and draws it's triangles on screen.
 * 
 * This function
 * - converts object space to camera space (CameraTransform)
 * - converts camera space to screen space (3D to 2D)
 * - calls `DrawLine` to set the pixels on screen.
 * Parameters:
 * vertices: vector of the camera space vertices
 * normals: directions of the respective world space normals.
 */
void Renderer::DrawTriangles(const vector<vec3>* vertices, const mat4& world_transform, const vector<Material>* material_list, const vector<vec3>* edge_normals, bool draw_normals, vec3 edge_color, bool fill)

{
	//incase none were passed
	vector<Material> matlist(1,Material());
	if(!material_list){
		material_list = &matlist;
	}
	//if normals isn't supplied, give this iterator some garbage value (vertices->begin())
	vector<Material>::const_iterator mat_it = material_list->begin();
	vector<vec3>::const_iterator normal_it = edge_normals != NULL ? edge_normals->begin() : vertices->begin();
	for(auto it = vertices->begin(); it != vertices->end();){
		//get the next face
		vec4 vert1 = vec4(*it);
		vec4 vert2 = vec4(*(it+1));
		vec4 vert3 = vec4(*(it+2));
		it = it + 3;

		vec4 normCoor1, normCoor2;

		vec4 vn1 = vec4(*normal_it);
		vec4 vn2 = vec4(*(normal_it+1));
		vec4 vn3 = vec4(*(normal_it+2));
		normal_it += 3;

		Material mat1 = *mat_it;
		Material mat2 = *(mat_it+(material_list->size() == vertices->size()));
		Material mat3 = *(mat_it+2*(material_list->size() == vertices->size()));
		if(material_list->size() == vertices->size()){
			mat_it += 3;
		}
		/*
		TRANSFORMATIONS + PROJECTION ( Tc-1 * v)
		*/
		vert1 = (mat_transform_inverse * world_transform * vert1);
		vert2 = (mat_transform_inverse * world_transform * vert2);
		vert3 = (mat_transform_inverse * world_transform * vert3);
		
		vec3 screenvert1 = toEuclidian(mat_project * vert1);
		vec3 screenvert2 = toEuclidian(mat_project * vert2);
		vec3 screenvert3 = toEuclidian(mat_project * vert3);
		if(screenvert1.z < -1 || screenvert1.z > 1 || screenvert2.z < -1 || screenvert2.z > 1 || screenvert3.z < -1 || screenvert3.z > 1){
			continue;
		}
		if(fill && calculateNormal(toVec3(screenvert1),toVec3(screenvert2),toVec3(screenvert3)).z > 0){
			continue;
		}
		
		//BackFace culling (currently not done in wireframe mode)
		vec3 norm_dir = calculateNormal(toVec3(vert1),toVec3(vert2),toVec3(vert3))/5.f;
		normCoor1 = (vert1 + vert2 + vert3) / 3;
		normCoor2 = normCoor1 + norm_dir;

		if(edge_normals != NULL){
			
			vn1 = mat_transform_inverse * world_transform * vn1;
			vn2 = mat_transform_inverse * world_transform * vn2;
			vn3 = mat_transform_inverse * world_transform * vn3;
		}
		else{
			//Use the face normal if no edge normals exist
			//(If we want to compute them better, do it in MeshModel.cpp)
			vn1 = vec4(0.5* norm_dir) + vert1;
			vn2 = vec4(0.5* norm_dir) + vert2;
			vn3 = vec4(0.5* norm_dir) + vert3;
		}

		//sometimes a point will get sent really far (matrix bs)
		//the DrawLine function wont draw out of bounds, but it will take
		//very long to go over the whole distance (~200,000 iterations).
		if (length(vert1) > LINE_TOO_LARGE || length(vert2) > LINE_TOO_LARGE || length(vert3) > LINE_TOO_LARGE)
		{
			continue;
		}

		if (fill) {
			FillPolygon(toVec3(vert1), toVec3(vert2), toVec3(vert3), toVec3(vn1), toVec3(vn2), toVec3(vn3), mat1,mat2,mat3);
		} else {
			
			/*
			Cameraspace coordinates to clipslace coordinates
			*/
			vert1 = toEuclidian(mat_project * vert1);
			vert2 = toEuclidian(mat_project * vert2);
			vert3 = toEuclidian(mat_project * vert3);

			/*
			Clipspace coordinates to screenspace coordinates
			*/
			vec3 p1 = vec3(RANGE(screenvert1.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(screenvert1.y,-1,1,0,m_height), screenvert1.z);
			vec3 p2 = vec3(RANGE(screenvert2.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(screenvert2.y,-1,1,0,m_height), screenvert2.z);
			vec3 p3 = vec3(RANGE(screenvert3.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(screenvert3.y,-1,1,0,m_height), screenvert3.z);

			DrawLine(p1, p2, edge_color);
			DrawLine(p2, p3, edge_color);
			DrawLine(p3, p1, edge_color);

			if(edge_normals != NULL){
				vn1 = toEuclidian(mat_project * vn1);
				vn2 = toEuclidian(mat_project * vn2);
				vn3 = toEuclidian(mat_project * vn3);
				vec3 a1 = vec3(RANGE(vn1.x,-aspect_ratio, aspect_ratio, 0, m_width), RANGE(vn1.y,-1,1,0,m_height), vn1.z);
				vec3 a2 = vec3(RANGE(vn2.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(vn2.y,-1,1,0,m_height), vn2.z);
				vec3 a3 = vec3(RANGE(vn3.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(vn3.y,-1,1,0,m_height), vn3.z);
				DrawLine(p1, a1, vec3(0, 0, edge_color.z));
				DrawLine(p2, a2, vec3(0, 0, edge_color.z));
				DrawLine(p3, a3, vec3(0, 0, edge_color.z));
			}
			//Normal:
			if(draw_normals){
				
				normCoor1 = toEuclidian(mat_project * normCoor1);
				normCoor2 = toEuclidian(mat_project * normCoor2);

				vec3 n1 = vec3(RANGE(normCoor1.x,-aspect_ratio,aspect_ratio, 0, m_width), RANGE(normCoor1.y, -1, 1, 0, m_height), normCoor1.z);
				vec3 n2 = vec3(RANGE(normCoor2.x,-aspect_ratio,aspect_ratio, 0, m_width), RANGE(normCoor2.y, -1, 1, 0, m_height), normCoor2.z);

			  	DrawLine(n1, n2, vec3(1, 0, 1));
    		}
		}
	}
}

void Renderer::RenderSuperBuffer()
{
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			// Render the scene at the supersampled resolution
			RenderPixel(x, y);
		}
	}

	// Downsample the supersampled buffer to the screen buffer
	DownsampleBuffer();
}

void Renderer::DownsampleBuffer()
{
	float epsilon = 1e-5;
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			vec3 accumulatedColor(0.0f, 0.0f, 0.0f);
			//float accumulatedDepth = 0.0f;

			int validSamples = 0;
			// Accumulate colors from the supersampled buffer
			for (int i = 0; i < supersample_factor; i++)
			{
				for (int j = 0; j < supersample_factor; j++)
				{
					int sx = x * supersample_factor + i;
					int sy = y * supersample_factor + j;

					if (sx < 0 || sx >= supersampled_width || sy < 0 || sy >= supersampled_height)
					{
						// Skip this sample if it's outside the valid range
						std::cout << "SOMETHING IS WRONG" << std::endl;
						continue;
					}

					accumulatedColor += m_supersampledBuffer[SUPER_INDEX(supersampled_width,sx,sy)];
					//accumulatedDepth += m_supersampledDepth[sx][sy];
					validSamples++;
					
				}
			}

			if (validSamples > 0)
			{
				accumulatedColor /= validSamples;
				//accumulatedDepth /= validSamples;
			}

			// Draw the pixel to the screen buffer
			m_outBuffer[INDEX(m_width, x, y, 0)] = accumulatedColor.x;	m_outBuffer[INDEX(m_width, x, y, 1)] = accumulatedColor.y;	m_outBuffer[INDEX(m_width, x, y, 2)] = accumulatedColor.z;
			
			/*
			vec3 finalColor = vec3(
				m_outBuffer[INDEX(m_width, x, y, 0)],
				m_outBuffer[INDEX(m_width, x, y, 1)],
				m_outBuffer[INDEX(m_width, x, y, 2)]
			);

			if (std::abs(accumulatedColor.x - finalColor.x) > epsilon ||
				std::abs(accumulatedColor.y - finalColor.y) > epsilon ||
				std::abs(accumulatedColor.z - finalColor.z) > epsilon) {
				std::cout << "accumulated color in downsample buffer = " << accumulatedColor << std::endl;
				std::cout << "after putting the color, the color in m_out buffer is: " << vec3(m_outBuffer[INDEX(m_width, x, y, 0)],
					m_outBuffer[INDEX(m_width, x, y, 1)],
					m_outBuffer[INDEX(m_width, x, y, 2)]) << std::endl;
			}
			*/
			
		}
	}
}

void Renderer::RenderPixel(int x, int y)
{
	for (int i = 0; i < supersample_factor; i++)
	{
		for (int j = 0; j < supersample_factor; j++)
		{
			int sx = x * supersample_factor + i;
			int sy = y * supersample_factor + j;

			if (sx < 0 || sx >= supersampled_width || sy < 0 || sy >= supersampled_height)
			{
				// Skip this sample if it's outside the valid range
				continue;
			}

			int sampleX = min(x + i, m_width - 1);
			int sampleY = min(y + j, m_height - 1);

			m_supersampledBuffer[SUPER_INDEX(supersampled_width,sx,sy)] += vec3(
				m_outBuffer[INDEX(m_width, sampleX, sampleY, 0)],
				m_outBuffer[INDEX(m_width, sampleX, sampleY, 1)],
				m_outBuffer[INDEX(m_width, sampleX, sampleY, 2)]
			);
			//m_supersampledDepth[sx][sy] += m_zbuffer[Z_INDEX(m_width, sampleX, sampleY)];
		}
	}
}

void Renderer::setAntiAliasing(bool new_anti_aliasing)
{
	anti_aliasing = new_anti_aliasing;
	ResizeBuffers(m_downsize_width,m_downsize_height);
}

bool Renderer::getAntiAliasingFlag()
{
	return anti_aliasing;
}

/*
* Lecture 4 slide 26-27.
* Given a triangle and a point, return 3 floats, representing the point as an average of the 3 points
* Returns: vec3: wp1 wp2 wp3
*/
vec3 getBarycentricCoordinates(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3){
	//More optimized implementation taken from : https://gamedev.stackexchange.com/a/116304
	float invDET = 1./((p2.y-p3.y) * (p1.x-p3.x) + 
                   (p3.x-p2.x) * (p1.y-p3.y));

	float l1 = ((p2.y-p3.y) * (p.x-p3.x) + (p3.x-p2.x) * (p.y-p3.y)) * invDET; 
	float l2 = ((p3.y-p1.y) * (p.x-p3.x) + (p1.x-p3.x) * (p.y-p3.y)) * invDET; 
	float l3 = 1. - l1 - l2;
	return vec3(l1, l2, l3);
}

vec3 Renderer::phongIllumination(const vec3& surface_point, const vec3& surface_normal, Material material)
{
	vec3 ambient_color(0.0f, 0.0f, 0.0f);
	vec3 diffuse_color(0.0f, 0.0f, 0.0f);
	vec3 specular_color(0.0f, 0.0f, 0.0f);
	vec3 view_direction = -surface_point;
	//use a reference to the vector rather than a pointer
	std::vector<Light*>& lights_ref = *lights;
	
	//renderer stores a constant ambient light
	ambient_color += ambient_light.getColor() * material.color_ambient * ambient_light.getIntensity();
	for (auto& light : lights_ref) 
	{
		// Ambient component
		if(dynamic_cast<AmbientLight*>(light)){
			ambient_color += light->getColor() * material.color_ambient * light->getIntensity();
			continue;
		}
		vec3 light_direction;
		PointLight* plight = dynamic_cast<PointLight*>(light);
		if(plight){
		//This might be wrong, we might wanna do our calculations in world space, not clip space(based on other students)
		//Would be faster to do this outside of this function
			vec3 light_position = toVec3(mat_transform_inverse * vec4(plight->getPosition()));
			light_direction = normalize(light_position - surface_point);
		}
		DirectionalLight* dlight = dynamic_cast<DirectionalLight*>(light);
		if(dlight){	
			//THIS MIGHT BE WRONG
			vec3 dir_point  = toVec3(mat_transform_inverse * vec4(surface_point+dlight->getDirection()));
			vec3 origin_point = toVec3(mat_transform_inverse * vec4(surface_point));
			light_direction = normalize(dir_point - origin_point);
		}

		// Diffuse component
		float cos_theta = max(0.0f, dot(surface_normal, light_direction));
		diffuse_color = diffuse_color + material.color_diffuse * light->getColor() * light->getIntensity() * cos_theta;

		// Specular component
		vec3 reflection_direction = reflect(-light_direction, surface_normal);
		float cos_phi = max(0.0f, dot(normalize(reflection_direction), normalize(view_direction)));
		specular_color = specular_color + material.color_specular * light->getColor() * light->getIntensity() * std::pow(cos_phi, material.k_shiny);
		}

	vec3 total_color = (ambient_color + diffuse_color + specular_color);
	return total_color.clamp(0.0f, 1.0f);
}


void Renderer::changeShadingMethod()
{
	switch(shading_method){
		case FLAT:
			shading_method = GOURAUD;
			break;
		case GOURAUD:
			shading_method = PHONG;
			break;
		default:
			shading_method = FLAT;
			break;
	}
}

void Renderer::FillPolygon(const vec3& vert1, const vec3& vert2, const vec3& vert3, const vec3& vn1, const vec3& vn2, const vec3& vn3, const Material& mat1, const Material& mat2, const Material& mat3)
{
	/*
	Cameraspace coordinates to clipslace coordinates
	*/
	vec3 screenvert1 = toEuclidian(mat_project * vert1);
	vec3 screenvert2 = toEuclidian(mat_project * vert2);
	vec3 screenvert3 = toEuclidian(mat_project * vert3);

	vec3 screenvn1 = toEuclidian(mat_project * vn1);
	vec3 screenvn2 = toEuclidian(mat_project * vn2);
	vec3 screenvn3 = toEuclidian(mat_project * vn3);

	float aspect_ratio = (float)(m_width) / (float)(m_height);
	vec3 p1 = vec3(RANGE(screenvert1.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(screenvert1.y, -1, 1, 0, m_height), screenvert1.z);
	vec3 p2 = vec3(RANGE(screenvert2.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(screenvert2.y, -1, 1, 0, m_height), screenvert2.z);
	vec3 p3 = vec3(RANGE(screenvert3.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(screenvert3.y, -1, 1, 0, m_height), screenvert3.z);

	const std::vector<vec3> vertices = { p1, p2, p3 };
	// Find the minimum and maximum y-coordinates to determine the scanline range
	int minY = static_cast<int>(floor(min( p1.y, p2.y)));
	minY = static_cast<int>(floor(min(minY, p3.y)));
	int maxY = static_cast<int>(ceil(max( p1.y, p2.y)));
	maxY = static_cast<int>(ceil(max( maxY, p3.y )));

	//Calculate colors once per polygon for FLAT and GOURAUD
	vec3 color1 = vec3(0,0,0);
	vec3 color2 = vec3(0,0,0);
	vec3 color3 = vec3(0,0,0);
	if(shading_method == FLAT){
		color1 = phongIllumination(0.33*vert1 + 0.33*vert2 +0.33*vert3, -calculateNormal(vert1,vert2,vert3), mat1);	//mat1 should be mat1,mat2,mat3,0.33,0.33,0.34
	}
	if(shading_method == GOURAUD){
		color1 = phongIllumination(vert1, normalize(vn1-vert1), mat1);
		color2 = phongIllumination(vert2, normalize(vn2-vert2), mat2);
		color3 = phongIllumination(vert3, normalize(vn3-vert3), mat3);
	}
	// Iterate through each scanline
	for (int y = max(0, minY); y <= min(m_height - 1, maxY); y++)
	{
		std::vector<int> intersections;

		// Check for intersections with each polygon edge
		for (int i = 0; i < vertices.size(); i++)
		{
			const vec3& p1 = vertices[i];
			const vec3& p2 = vertices[(i + 1) % vertices.size()];

			if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) 
			{
				// Calculate the intersection point's x-coordinate
				int xIntersection = static_cast<int>(p1.x + (static_cast<double>(y - p1.y) / (p2.y - p1.y)) * (p2.x - p1.x));
				intersections.push_back(xIntersection);
			}
		}

		// Sort the intersection points in ascending order
		BubbleSort(intersections);

		// Fill the pixels between pairs of intersections
		for (int i = 0; i < intersections.size(); i += 2) 
		{
			int startX = max(0, intersections[i]);
			int endX = min(m_width, intersections[i + 1]); 

			
			for (int x = startX; x <= endX; x++) {
				// Get the weights of the three edges of the tri
				vec3 weights = getBarycentricCoordinates(vec2(x, y), vec2(p1.x, p1.y), vec2(p2.x, p2.y), vec2(p3.x, p3.y));
				
				// Calculate the current Z
				vec3 surface_point = weights.x * vert1 + weights.y * vert2 + weights.z * vert3;	//Not efficient but easy to work with
				float screen_z = weights.x * screenvert1.z + weights.y * screenvert2.z + weights.z * screenvert3.z;
				//dont do color calculations for covered pixels!!!
				if(screen_z >= m_zbuffer[Z_INDEX(m_width, x, y)]) {
					continue;
				}

				// Calculate the current norm
				vec3 norm = weights.x * (vn1-vert1) + weights.y * (vn2-vert2) + weights.z * (vn3-vert3);
				norm = normalize(norm);

				// Calculate the pixel based on shading method
				vec3 pixel_color = vec3(0,0,0);
				switch(shading_method){
					case FLAT:
						pixel_color = color1;
						break;
					case GOURAUD:
						pixel_color = weights.x * color1 + weights.y * color2 + weights.z * color3;
						break;
					case PHONG:
						//Material mat = Material::weightedAverage(mat1,mat2,mat3,weights.x,weights.y,weights.z);
						Material mat = mat1;
						pixel_color = phongIllumination(surface_point, norm, mat);
						break;
				}
				//Blend the color with fog
				if(draw_fog){
					pixel_color = blendWithFogs(surface_point, pixel_color);
				}
				DrawPixel(x, y, screen_z, pixel_color);
      		}
		}
	}
}

vec2 normalizeVectorWithFixedPoint(const vec2& fixedPoint, const vec2& pointToNormalize)
{
	float directionX = pointToNormalize.x - fixedPoint.x;
	float directionY = pointToNormalize.y - fixedPoint.y;

	float length = std::sqrt(directionX * directionX + directionY * directionY);

	// Check if the length is not zero to avoid division by zero
	if (length > 0.0f) {
		// Normalize the direction vector and scale it to the original length
		return vec2(fixedPoint.x + directionX / length, fixedPoint.y + directionY / length);
	}

	// Return the original point if the length is zero
	return pointToNormalize;
}

void Renderer::DrawNormalsToVertices(const vector<vec3>* vertices, const vector<vec3>* vertex_normals, bool draw_normals)
{
	if (!vertex_normals) {
		return;
	}

	//if normals isn't supplied, give this iterator some garbage value (vertices->begin())
	vector<vec3>::const_iterator normal =  vertex_normals->begin();
	for (auto it = vertices->begin(); it != vertices->end(); it++, normal++) {
        // Get the next face
        vec4 vert1 = vec4(*it);
        vec4 normCoor = vec4(*normal);

        /*
        TRANSFORMATIONS + PROJECTION (P * Tc-1 * v)
        */

        vert1 = toEuclidian(mat_project * (mat_transform_inverse * vert1));
        normCoor = toEuclidian(mat_project * (mat_transform_inverse * (normCoor+vert1)));

        // Normalize the vector without applying the range
		//vec2 normalized_end_point = normalize(vec2(normCoor.x,normCoor.y)) + vec2(vert1.x,vert1.y);
		
		// Scale down the normalized vector (make the normals smaller)

		
		// Apply the range to the normalized point

		vec3 first_point = vec3(RANGE(normCoor.x,-aspect_ratio,aspect_ratio, 0, m_width), RANGE(normCoor.y, -1, 1, 0, m_height), normCoor.z);

		// Normal:
		if (draw_normals) {
			DrawLine(first_point, vec3(RANGE(vert1.x,-aspect_ratio,aspect_ratio, 0, m_width), RANGE(vert1.y, -1, 1, 0, m_height), vert1.z), vec3(0.2,0.5,1));

		}
    }
}

void Renderer::DrawBoundingBox(const vec3* bounding_box, const mat4& world_transform, bool draw_box) 
{
	if (!bounding_box || !draw_box) {
		return;
	}
	
	vec4 new_bounding_box[8];
	vec2 bounding_box_in_vectwo[8];
	for (int i = 0; i < 8; i++) {
		// Convert 3D point to homogeneous coordinates
		vec4 homogeneous_point = vec4(bounding_box[i], 1.0f);
		
		

		// Apply transformations
		new_bounding_box[i] = toEuclidian(mat_project * (mat_transform_inverse * world_transform * homogeneous_point));
		bounding_box_in_vectwo[i] = vec2(RANGE(new_bounding_box[i].x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(new_bounding_box[i].y, -1, 1, 0, m_height));
		//bounding_box_in_vectwo[i] = vec2(new_bounding_box[i].x, new_bounding_box[i].y);

	}
// Define the indices to connect vertices in a sequential manner
	const int indices[12][2] = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0},
		{4, 5}, {5, 7}, {7, 6}, {6, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	// Draw lines to connect the vertices of the bounding box using the defined indices
	// Draw lines to connect the vertices of the bounding box using the defined indices
	for (int i = 0; i < 12; ++i) {
		if (new_bounding_box[indices[i][0]].z < -1 || new_bounding_box[indices[i][0]].z > 1 ||
			new_bounding_box[indices[i][1]].z < -1 || new_bounding_box[indices[i][0]].z > 1) {
			continue;
		}

		// Interpolate Z-values
		float z1 = new_bounding_box[indices[i][0]].z;
		float z2 = new_bounding_box[indices[i][1]].z;

		// Draw line with Z-buffer check
		DrawLine(vec3(bounding_box_in_vectwo[indices[i][0]], z1), vec3(bounding_box_in_vectwo[indices[i][1]], z2), vec3(1, 1, 0));

	}
}

void Renderer::DrawSymbol(const vec3& vertex, const mat4& world_transform, SYMBOL_TYPE symbol, float scale,vec3 colors)
{
	scale *= 4;
	const std::vector<vec2> square_shape = { vec2(-1, -1), vec2(1, -1), vec2(1, -1), vec2(1, 1), vec2(1, 1), vec2(-1, 1), vec2(-1, 1), vec2(-1, -1) };
	const std::vector<vec2> x_shape = { vec2(-1, -1), vec2(1, 1), vec2(1, -1), vec2(-1, 1) };
	const std::vector<vec2> star_shape = { vec2(0, 1), vec2(0, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1), vec2(-1, -1), vec2(-1, 0), vec2(1, 0) };
	const std::vector<vec2> plus_shape = { vec2(0, 1), vec2(0, -1), vec2(-1, 0), vec2(1, 0) };

	const std::vector<vec2>* decided = &square_shape;
	
	vec4 screen_space = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vec4(vertex, 1.0f)));
	vec2 image_space = vec2(RANGE(screen_space.x,-aspect_ratio,aspect_ratio, 0, m_width), RANGE(screen_space.y, -1, 1, 0, m_height));
	switch (symbol) {
	case SYM_SQUARE:
		decided = &square_shape;
		break;
	case SYM_X:
		decided = &x_shape;
		break;
	case SYM_STAR:
		decided = &star_shape;
		break;
	case SYM_PLUS:
		decided = &plus_shape;
		break;
	default:
		decided = &x_shape;
		break;
	}

	auto a = decided->begin();
	while (a != decided->end()) {
		// Draw line with Z-buffer check
		DrawLine(vec3(scale * (*a) + image_space, screen_space.z), vec3(scale * (*(a + 1)) + image_space, screen_space.z), colors);
		a += 2;

	}
}

void Renderer::DrawLightSymbol(Light* light){
	PointLight* plight = dynamic_cast<PointLight*>(light);
	if(plight){
		DrawSymbol(plight->getPosition(),mat4(),SYM_STAR, 1);
		return;

	}
	DirectionalLight* dlight = dynamic_cast<DirectionalLight*>(light);
	if(dlight){
		vec3 v1 = 0.9 * dlight->getDirection();
		vec3 v2 = v1 - 0.1*dlight->getDirection();
		
		vec3 p1 = CLIP_TO_SCREEN(v1.x,v1.y,v1.z);
		vec3 p2 = CLIP_TO_SCREEN(v2.x,v2.y,v2.z);
		DrawLine(p1,p2,dlight->getColor());
		DrawLine(p1-vec3(3,3,0),p2-vec3(3,3,0),dlight->getColor());
		DrawLine(p1+vec3(3,3,0),p2+vec3(3,3,0),dlight->getColor());
		return;
	}

}


vec3 Renderer::GetWorldPosition(int x, int y)
{
	// Ensure x and y are within bounds
	if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
		return vec3(0.0f, 0.0f, 0.0f);  // Return a default value or handle the out-of-bounds case
	}

	// Retrieve the depth value from the depth buffer
	float depth_value = m_zbuffer[Z_INDEX(m_width, x, y)];

	// Normalize the depth value to [0, 1]
	float normalized_depth = depth_value / far_z;

	// Calculate normalized device coordinates (NDC) from pixel coordinates
	float ndc_x = (2.0f * x) / m_width - 1.0f;
	float ndc_y = 1.0f - (2.0f * y) / m_height;

	// Create a homogeneous coordinates vector
	vec4 homogeneous_coords(ndc_x, ndc_y, normalized_depth, 1.0f);

	// Calculate the inverse view-projection matrix
	mat4 mat_view_projection_inverse = inverse(mat_project * mat_transform_inverse);

	// Transform the homogeneous coordinates to view space
	vec4 view_coords = mat_view_projection_inverse * homogeneous_coords;

	// Divide by the w component to get the view space coordinates
	vec3 view_position = toVec3(view_coords) / view_coords.w;

	// The view position is now in view space. To get the world position, you can transform it by the inverse view matrix
	vec4 world_coords = mat_transform_inverse * vec4(view_position, 1.0f);

	return toVec3(world_coords);
}

void Renderer::setFogFlag(bool set_fog)
{
	draw_fog = set_fog;
}

bool Renderer::getFogFlag()
{
	return draw_fog;
}

// Mix function for linear interpolation between two values

vec3 mix(const vec3& x, const vec3& y, float a)
{
	return x * (1.0f - a) + y * a;
}

vec3 Renderer::ComputeFogColor(const Fog& fog, int x, int y)
{
	vec3 dir = GetWorldPosition(x, y);
	float dist = length(dir);
	float fog_amount = 0.5*fog.ComputeFog(dist);

	vec3 pixel_color = vec3(m_outBuffer[INDEX(m_width, x, y, 0)], m_outBuffer[INDEX(m_width, x, y, 1)], m_outBuffer[INDEX(m_width, x, y, 2)]);

	return mix(pixel_color, fog.getFogColor(), fog_amount);
}

void Renderer::ApplyFog(const Fog& fog)
{
	if (!draw_fog) {
		return;  // Do nothing if draw_fog is false
	}

	// Iterate through each pixel in the buffer
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			// Get the pixel's depth from the z-buffer
			float pixel_depth = m_zbuffer[Z_INDEX(m_width, x, y)];

			// Check if the pixel is within the fog range
			if (pixel_depth >= fog.getFogStart() && pixel_depth <= fog.getFogEnd())
			{
				// Interpolate between the pixel color and the fog color based on the fog factor
				vec3 fogged_color = ComputeFogColor(fog, x, y);

				// Update the pixel color in the buffer
				m_outBuffer[INDEX(m_width, x, y, 0)] = fogged_color.x;
				m_outBuffer[INDEX(m_width, x, y, 1)] = fogged_color.y;
				m_outBuffer[INDEX(m_width, x, y, 2)] = fogged_color.z;
			}
		}
	}
}


vec3 Renderer::blendWithFogs(const vec3& surface_point, const vec3& pixel_color){
	//This isn't correct for multiple fogs
	std::vector<Fog*>& fogs_ref = *fogs;
	vec3 fogged_color;
	for (auto& fog : fogs_ref) {
		float fog_amount = fog->ComputeFog(-surface_point.z);
		fogged_color = mix(fog->getFogColor(),pixel_color,fog_amount);
	}
	return fogged_color;
}

void Renderer::SetCameraTransformInverse(const mat4& cTransform){
	mat_transform_inverse = cTransform;
}
void Renderer::SetProjection(const mat4& projection){
	mat_project = projection;
}

void Renderer::setCameraMatrixes(const mat4& cTransformInverse, const mat4& Projection){
	SetCameraTransformInverse(cTransformInverse);
	SetProjection(Projection);
}

void Renderer::setCameraMatrixes(Camera* camera){
	setCameraMatrixes(camera->getTransformInverse(), camera->getProjection());
}


void Renderer::Init(){
	CreateBuffers(m_width,m_height);

}


/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );

	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_downsize_width, m_downsize_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_downsize_width, m_downsize_height);
}

void Renderer::SwapBuffers()
{
	UpdateBuffer();
	//clear the new buffer
	ClearBuffer();
}

void Renderer::DownsizeBuffer(){
	std::cout << "- DOWNSIZING " << m_width << "->" << m_downsize_width << std::endl;
	// Iterate every pixel
	for(int i = 0; i < m_downsize_width; i++){
		for(int j = 0; j < m_downsize_height; j ++ ){

			float r = 0,g = 0,b = 0;
			for(int sx = 0; sx < supersample_factor; sx ++){
				for(int sy = 0; sy < supersample_factor; sy++){
					r += m_outBuffer[INDEX(m_width,sx+i*supersample_factor,sy+j*supersample_factor,0)];
					g += m_outBuffer[INDEX(m_width,sx+i*supersample_factor,sy+j*supersample_factor,1)];
					b += m_outBuffer[INDEX(m_width,sx+i*supersample_factor,sy+j*supersample_factor,2)];
				}
			}
			int area = supersample_factor * supersample_factor;
			m_downsizeBuffer[INDEX(m_downsize_width,i,j,0)] = r / (area);
			m_downsizeBuffer[INDEX(m_downsize_width,i,j,1)] = g / (area);
			m_downsizeBuffer[INDEX(m_downsize_width,i,j,2)] = b / (area);

		}
	}
}
//Doesn't clear the buffer afterwards!
void Renderer::UpdateBuffer(){

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	if(anti_aliasing){
		
		std::cout << "-V DRAWING antialiased " << m_outBuffer << "," << m_downsizeBuffer << std::endl;
		DownsizeBuffer();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_downsize_width, m_downsize_height, GL_RGB, GL_FLOAT, m_downsizeBuffer);
	}
	else{
		
		std::cout << "-N DRAWING normal " << m_outBuffer << "," << m_downsizeBuffer << std::endl;
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();

}