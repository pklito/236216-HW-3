#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
using namespace std;

class Camera;

typedef enum {
	SYM_STAR,
	SYM_SQUARE,
	SYM_X,
	SYM_PLUS
} SYMBOL_TYPE;

class Renderer
{
	float* m_outBuffer; // 3*width*height
	float* m_zbuffer; // width*height
	int m_width, m_height;
	mat4 mat_transform_inverse;
	mat4 mat_project;

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
	void FillBuffer(float r, float g, float b);
	void FillEdges(float amount, float r, float g, float b);
	void ResizeBuffers(int new_width, int new_height);
	void Init();
	void DrawLine(vec2 vert1, vec2 vert2, float r = 1, float g = 1, float b = 1);
	void DrawTriangles(const vector<vec3>* vertices, const mat4& world_transform, const vector<vec3>* edge_normals = NULL, bool draw_normals = false, float r = 1, float g = 1, float b = 1);
	void DrawNormalsToVertices(const vector<vec3>* vertices, const vector<vec3>* vertex_normals = NULL, bool draw_vertex_normals = false);

	void DrawBoundingBox(const vec3* bounding_box, const mat4& world_transform, bool draw_box = false);
	void DrawPixel(int x, int y, float r, float g, float b);
	void DrawPixelSafe(int x, int y, float r, float g, float b);
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
