#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "InitShader.h"
#include <map>
#include "util.h"

class Camera;

class Program
{
private:
	std::map<const char *, GLuint> uniform_locs;

	GLuint try_add(const char *uniform)
	{
		GLuint loc = glGetUniformLocation(program, uniform);
		if (loc == (GLuint)(-1))
		{
			std::cout << "[X] " << program << " failed to find uniform: " << uniform << std::endl;
		}
		uniform_locs[uniform] = loc;
		return loc;
	}

public:
	GLuint program;
	Program() : uniform_locs(), program(){};
	template <typename... Args>
	Program(const char *vshader, const char *fshader, Args... args)
	{
		program = InitShader(vshader, fshader);
		std::cout << "Created program " << program << " : " << vshader << ", " << fshader << std::endl;
		for (const char *uniform : {args...})
		{
			try_add(uniform);
		}
	}
	GLuint find(const char *uniform)
	{
		auto it = uniform_locs.find(uniform);

		if (it != uniform_locs.end())
		{
			return it->second;
		}
		else
		{
			return try_add(uniform);
		}
	}
	void Delete()
	{
		glDeleteProgram(program);
	}
};

class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer;	// width*height
	int m_width, m_height;

	// vertex/fragment shader
	Program program_wireframe;
	std::vector<Program> programs;
	int current_program;

	// current camera storage
	mat4 mat_transform_inverse;
	mat4 mat_project;
	vec3 camera_position;

	//lights
	std::vector<Light*>* lights;
	AmbientLight ambient_light;

	void CreateBuffers(int width, int height);
	
	void _passLights(Program & program);
	void _DrawTris(Program &program, GLuint vao, GLuint face_count, const mat4 &wm_transform, const mat4 &wm_normal_transform);
	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height, const char *vshader, const char *fshader);
	~Renderer(void);
	void Init();

	void StartDraw();
	void EndDraw();

	void CreateProgram(const char *vshader, const char *fshader);
	void RemoveProgram(int index);

	void DrawMesh(GLuint vao, GLuint face_count, const mat4 &transform = mat4(), const mat4 &normal_transform = mat4());
	void DrawWireframe(GLuint vao, GLuint face_count, const mat4 &wm_transform);
	void DrawLines(GLuint lines_vao, GLuint lines_count, const mat4 &wm_transform, vec3 color = vec3(1, 0, 1));

	void SetCameraTransformInverse(const mat4 &cTransform);
	void SetProjection(const mat4 &projection);
	void setCameraMatrixes(const mat4 &cTransform, const mat4 &Projection, const vec3 &camera_pos);
	void setCameraMatrixes(Camera *camera);
	
	void setLights(std::vector<Light*>* lights) {this->lights = lights;};
	void setAmbientLight(const AmbientLight& light) {ambient_light = light;};
	AmbientLight getAmbientLight() {return ambient_light;};

	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
};
