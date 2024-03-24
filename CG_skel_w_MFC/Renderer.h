#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

class Camera;

class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;

	//vertex/fragment shader
	GLuint program;

	//current camera storage
	mat4 mat_transform_inverse;
	mat4 mat_project;
	vec3 camera_position;

	void CreateBuffers(int width, int height);
	void CreateProgram(const char* vshader, const char* fshader);
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
	Renderer(int width, int height, const char* vshader, const char* fshader);
	~Renderer(void);
	void Init();
	void DrawMesh(GLuint vao,GLuint face_count, const mat4& transform);
	void SetCameraTransformInverse(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void setCameraMatrixes(const mat4& cTransform, const mat4& Projection);
	void setCameraMatrixes(Camera* camera);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
};
