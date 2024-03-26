#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

class Camera;

class Program {
	private:
		GLuint get_uniform(GLuint program, const char* name){
			return glGetUniformLocation(program, name);
		}
		
		
	public:
		GLuint program;
		GLuint uniform_loc1;
		GLuint uniform_loc2;
		GLuint uniform_loc3;
	Program() : program(0), uniform_loc1(-1), uniform_loc2(-1), uniform_loc3(-1) {};
	Program(GLuint programID, GLuint uniformLocation1, GLuint uniformLocation2, GLuint uniformLocation3)
        : program(programID), uniform_loc1(uniformLocation1), uniform_loc2(uniformLocation2), uniform_loc3(uniformLocation3) {};
	Program(GLuint programID,const char* name1,const char* name2,const char* name3) : program(programID){
		uniform_loc1=get_uniform(program,name1);
		uniform_loc2=get_uniform(program,name2);
		uniform_loc3=get_uniform(program,name3);
	};
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
