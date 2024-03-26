#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "InitShader.h"

class Camera;

class Program {
	public:
		GLuint program;
		std::vector<GLuint> uniform_locs;
	template<typename... Args>
	Program(const char* vshader, const char* fshader, Args ... args){
		program = InitShader(vshader,fshader);
		for(const char* uniform : {args...}){
			GLuint loc = glGetUniformLocation(program, uniform);
			if(loc == (GLuint)(-1)){
				std::cout << "[X] " << vshader << " failed to find uniform: " << uniform << std::endl;
			}
			uniform_locs.push_back(loc);
		}
	}
	void Delete(){
		glDeleteProgram(program);
	}	
};

class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;

	//vertex/fragment shader
	std::vector<Program> programs;
	int current_program;

	//current camera storage
	mat4 mat_transform_inverse;
	mat4 mat_project;
	vec3 camera_position;

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
	Renderer(int width, int height, const char* vshader, const char* fshader);
	~Renderer(void);
	void Init();

	void StartDraw();
	void EndDraw();

	void CreateProgram(const char* vshader, const char* fshader);
	void RemoveProgram(int index);

	void DrawMesh(GLuint vao,GLuint face_count, const mat4& transform = mat4(), const mat4& normal_transform = mat4());
	void SetCameraTransformInverse(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void setCameraMatrixes(const mat4& cTransform, const mat4& Projection);
	void setCameraMatrixes(Camera* camera);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
};
