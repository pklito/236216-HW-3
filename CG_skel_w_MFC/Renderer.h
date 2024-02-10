#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

using namespace std;
class Renderer
{
	float* m_outBuffer; // 3*width*height
	float* m_zbuffer; // width*height
	int m_width, m_height;
	mat4 mat_transform_inverse;
	mat4 mat_project;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	void DrawLine(vec2 vert1, vec2 vert2, int special_color = 0);
	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals = NULL);
	void DrawBoundingBox(const vec3* bounding_box);
	void DrawPoint(const vec3& vertex);
	void SetCameraTransformInverse(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();

	void Render();
	void HandleInput();
};

#define CLAMP(x,min,max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

#define RANGE(x,min,max,new_min,new_max) (((((x)-(min))*((new_max)-(new_min))/((max)-(min))))+(new_min))