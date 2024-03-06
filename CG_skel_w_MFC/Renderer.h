#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "util.h"
using namespace std;

class Camera;

typedef enum {
	SYM_STAR,
	SYM_SQUARE,
	SYM_X,
	SYM_PLUS
} SYMBOL_TYPE;


enum ShadingMethod {
	BARYCENTRIC,
	PHONG
};


class Renderer
{
	float* m_outBuffer; // 3*width*height
	float* m_zbuffer; // width*height
	int m_width, m_height;
	float far_z = 100.0f;
	mat4 mat_transform_inverse;
	mat4 mat_project;
	vec3 camera_position;
	vector<Light*> lights;

	ShadingMethod shading_method;

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
	void FillBuffer(vec3 color);
	void FillEdges(float percent, vec3 color);
	void ResizeBuffers(int new_width, int new_height);
	void Init();
	void DrawLine(vec3 vert1, vec3 vert2, vec3 color = vec3(1,1,1));
	void DrawTriangles(const vector<vec3>* vertices, const mat4& world_transform, Material material, const vector<vec3>* edge_normals = NULL, bool draw_normals = false,vec3 edge_color = vec3(1,1,1), bool fill = false);
	vec3 getBarycentricCoordinates(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3);
	vec3 phongIllumination(const vec3& surface_point, const vec3& surface_normal, const mat4& world_transform, Material material, const vec3& color);
	void FillPolygon(const vec3& vert1, const vec3& vert2, const vec3& vert3, const mat4& world_transform, const vec3& color, Material material);


	vec3 calculateViewDirection(const vec3& surface_point, const mat4& world_transform);
	void DrawNormalsToVertices(const vector<vec3>* vertices, const vector<vec3>* vertex_normals = NULL, bool draw_vertex_normals = false);
	void setCameraPos(vec3 camera_pos);
	void addLight(Light* light);
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

#define CLAMP(x,min,max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define RANGE(x,min,max,new_min,new_max) (((((x)-(min))*((new_max)-(new_min))/((max)-(min))))+(new_min))
