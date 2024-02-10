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
	void DrawPixel(int x, int y, float r, float g, float b);
	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals=NULL);
	void DrawBoundingBox(const vec3* bounding_box);
	void DrawPoint(const vec3& vertex);
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