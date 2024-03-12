#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "util.h"
#include "Fog.h"
#include "GL/glew.h"

class Camera;

typedef enum {
	SYM_STAR,
	SYM_SQUARE,
	SYM_X,
	SYM_PLUS
} SYMBOL_TYPE;


enum ShadingMethod {
	FLAT,
	GOURAUD,
	PHONG
};


class Renderer
{
	float* m_outBuffer; // 3*width*height
	float* m_zbuffer; // width*height
	std::vector<std::vector<vec3>> m_supersampledBuffer;
	//std::vector<std::vector<float>> m_supersampledDepth;
	int supersample_factor = 2;
	int supersampled_width, supersampled_height;
	int m_width, m_height;
	float far_z = 20.0f;
	mat4 mat_transform_inverse;
	mat4 mat_project;
	vec3 camera_position;
	std::vector<Light*>* lights;
	AmbientLight ambient_light;
	std::vector<Fog*>* fogs;

	ShadingMethod shading_method;
	bool draw_fog;

	bool anti_aliasing;

	int curr_color;

	void CreateBuffers(int width, int height);
	void ReleaseBuffers();
	void CreateLocalBuffer();

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////

	void UpdateBuffer();
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);

	void ClearBuffer();
	void CreateSupersampledBuffer();
	void FillBuffer(vec3 color);
	void FillEdges(float percent, vec3 color);
	void ResizeBuffers(int new_width, int new_height);
	void Init();
	void DrawLine(vec3 vert1, vec3 vert2, vec3 color = vec3(1,1,1));

	void DrawTriangles(const std::vector<vec3>* vertices, const mat4& world_transform, const std::vector<Material>* materials, const std::vector<vec3>* edge_normals = NULL, bool draw_normals = false,vec3 edge_color = vec3(1,1,1), bool fill = false);
	vec3 phongIllumination(const vec3& surface_point, const vec3& surface_normal, Material material);
	void FillPolygon(const vec3& vert1, const vec3& vert2, const vec3& vert3, const vec3& vn1, const vec3& vn2, const vec3& vn3, const Material& mat1, const Material& mat2, const Material& mat3);
	void RenderSuperBuffer();

	vec3 GetWorldPosition(int x, int y);
	vec3 ComputeFogColor(const Fog& fog, int x, int y);
	void setFogFlag(bool fog);
	bool getFogFlag();
	void setFog(std::vector<Fog*>* fogs) { this->fogs = fogs; };
	void ApplyFog(const Fog& fog);
	vec3 blendWithFogs(const vec3& surface_point, const vec3& pixel_color);

	void DownsampleBuffer();
	void RenderPixel(int x, int y);

	void setAntiAliasing(bool new_anti_aliasing);
	bool getAntiAliasingFlag();
	void CheckColorDifferences(const std::vector<std::vector<vec3>>& supersampledBuffer, const float* finalBuffer, int width, int height);

	void DrawNormalsToVertices(const std::vector<vec3>* vertices, const std::vector<vec3>* vertex_normals = NULL, bool draw_vertex_normals = false);
	void setCameraPos(vec3 camera_pos);
	void setLights(std::vector<Light*>* lights) {this->lights = lights;};
	void setAmbientLight(const AmbientLight& light) {ambient_light = light;};
	AmbientLight getAmbientLight() {return ambient_light;};
	
	void changeShadingMethod();

	void DrawBoundingBox(const vec3* bounding_box, const mat4& world_transform, bool draw_box = false);
	void DrawPixel(int x, int y, float z, vec3 color);
	void DrawPixelSafe(int x, int y, float z, vec3 color);

	void DrawSymbol(const vec3& vertex ,const mat4& =  mat4(), SYMBOL_TYPE symbol = SYM_STAR, float scale = 1, vec3 colors = vec3(0.6, 0.9, 0.4));
	void SetCameraTransformInverse(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void setCameraMatrixes(const mat4& cTransform, const mat4& Projection);
	void setCameraMatrixes(Camera* camera);
	void SwapBuffers();
	//void ClearColorBuffer();
	//void ClearDepthBuffer();
	void SetDemoBuffer();
};

#define RANGE(x,min,max,new_min,new_max) (((((x)-(min))*((new_max)-(new_min))/((max)-(min))))+(new_min))
