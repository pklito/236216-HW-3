#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
using namespace std;

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
	float far_z = 100.0f;
	mat4 mat_transform_inverse;
	mat4 mat_project;

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
	void FillBuffer(float r, float g, float b);
	void FillEdges(float amount, float r, float g, float b);
	void ResizeBuffers(int new_width, int new_height);
	void Init();
	void DrawLine(vec3 vert1, vec3 vert2, float r = 1, float g = 1, float b = 1);
	void DrawTriangles(const vector<vec3>* vertices, const mat4& world_transform, const vector<vec3>* edge_normals = NULL, bool draw_normals = false, float r = 1, float g = 1, float b = 1, bool color = false);
	void FillPolygon(const vec3& p1, const vec3& p2, const vec3& p3, float r, float g, float b);

	void DrawNormalsToVertices(const vector<vec3>* vertices, const vector<vec3>* vertex_normals = NULL, bool draw_vertex_normals = false);

	void DrawBoundingBox(const vec3* bounding_box, const mat4& world_transform, bool draw_box = false);
	void DrawPixel(int x, int y, float z, float r, float g, float b);
	void DrawPixelSafe(int x, int y, float z, float r, float g, float b);
	void DrawSymbol(const vec3& vertex ,const mat4& =  mat4(), SYMBOL_TYPE symbol = SYM_STAR, float scale = 1, vec3 colors = vec3(0.6, 0.9, 0.4));
	void SetCameraTransformInverse(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void setCameraMatrixes(const mat4& cTransform, const mat4& Projection);
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

/*
Macro set pixels
#define SET_CHANNEL(array,x,y,width,channel,value) ((array)[INDEX((width),(x),(y),(channel))]=value)
#define SET_PIXEL(array,x,y,width,r,g,b)	SET_CHANNEL((array),(x),(y),(width),0,(r)); SET_CHANNEL((array),(x),(y),(width),1,(g)); SET_CHANNEL((array),(x),(y),(width),2,(b))

#define SET_CHANNEL_CLAMPED(array,x,y,width,height,channel,value) SET_CHANNEL(array,CLAMP((x),0,(width)),CLAMP((y),0,(height)),(width),(channel),(value))
#define SET_PIXEL_CLAMPED(array,x,y,width,height,r,g,b) SET_PIXEL((array),CLAMP((x),0,(width)),CLAMP((y),0,(height)),(width),(r),(g),(b))
*/