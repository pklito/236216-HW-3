#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include "scene.h"
#include "Texture.h"

#define INDEX(width, x, y, c) ((x) + (y) * (width)) * 3 + c
GLenum err;
#define DEBUG_BUG() while((err = glGetError()) != GL_NO_ERROR){ std::cerr << __FUNCTION__ << ":" << gluErrorString(err) << std::endl;}

Renderer::Renderer() : Renderer(512, 512, "vshader.glsl", "fshader.glsl")
{
}

Renderer::Renderer(int width, int height, const char *vshader, const char *fshader) : m_width(width), m_height(height), programs(), current_program(0)
{
	InitOpenGLRendering();
	CreateBuffers(width, height);
	CreateProgram(vshader, fshader);
	program_wireframe = Program("lines_vshader.glsl", "lines_fshader.glsl", "world_transform", "camera_transform", "color");
	program_texture = Program("minimal_vshader.glsl", "texture_fshader.glsl", "world_transform", "camera_transform");
}

Renderer::~Renderer(void)
{
	for (Program p : programs)
	{
		p.Delete();
	}
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); // Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
}

void Renderer::CreateProgram(const char *vshader, const char *fshader)
{
	programs.push_back(Program(vshader, fshader, "world_transform", "camera_transform", "normal_transform"));
}

void Renderer::RemoveProgram(int index)
{
	if (index >= programs.size())
		return;

	// Delete the shaders from the GPU
	glDeleteProgram(programs[index].program);

	// Remove from vector
	programs.erase(programs.begin() + index);

	// ensure we didn't erase the last program
	current_program %= programs.size();
}

void Renderer::SetDemoBuffer()
{
	// vertical line
	for (int i = 0; i < m_width; i++)
	{
		m_outBuffer[INDEX(m_width, 256, i, 0)] = 1;
		m_outBuffer[INDEX(m_width, 256, i, 1)] = 0;
		m_outBuffer[INDEX(m_width, 256, i, 2)] = 0;
	}
	// horizontal line
	for (int i = 0; i < m_width; i++)
	{
		m_outBuffer[INDEX(m_width, i, 256, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, 256, 1)] = 0;
		m_outBuffer[INDEX(m_width, i, 256, 2)] = 1;
	}
}

//
// Rendering
//
#define LIGHT_ARRAY_MAX 10
/// @brief Assumes glUseProgram happened
/// @param program 
void Renderer::_passLights(Program & program){
	//glUseProgram(program.program);

	// Create the buffers to send as uniforms
	GLfloat light_point_array[LIGHT_ARRAY_MAX*3*2];
	GLfloat directional_array[LIGHT_ARRAY_MAX*3*2];
	GLfloat ambient_array[3];

	// store which light we're sending currently
	int point_num = 0;
	int directional_num = 0;
	std::vector<Light*>& lights_ref = *lights;
	for(Light* light : lights_ref){
		// send the light data to the right buffer
		if(dynamic_cast<PointLight*>(light) && point_num < LIGHT_ARRAY_MAX){
			light->passArray(light_point_array + 6*point_num);
			point_num += 1;
		}

		if(dynamic_cast<DirectionalLight*>(light) && directional_num < LIGHT_ARRAY_MAX){
			light->passArray(directional_array + 6*directional_num);
			directional_num += 1;
		}
	}
	//only one ambient light, copy its colors onto the buffer
	ambient_light.passArray(ambient_array);

	//
	// Send the buffers to the shader!
	//
	glUniformMatrix2x3fv(program.find("point_lights"), point_num, GL_FALSE, light_point_array);
	glUniformMatrix2x3fv(program.find("directional_lights"), directional_num, GL_FALSE, directional_array);
	glUniform3fv(program.find("ambient_light"), 1, ambient_array);

}
void Renderer::StartDraw()
{
	//Choose the current program
	glUseProgram(programs[current_program].program);
	//Pass light sources

	//Send over the light sources
	_passLights(programs[current_program]);

	//Enable backface culling and Z buffering
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//Clear the buffers
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);	//Choose pixels with a smaller zvalue
}

void Renderer::EndDraw()
{
	glFlush();
	glutSwapBuffers();
	glUseProgram(0);
}

/// @brief Render a mesh with a certain program
/// @param program Program class
/// @param vao vao of the vertices
/// @param face_count amount of vertices/3
/// @param wm_transform world*model transform of the model
/// @param wm_normal_transform world*model transform of the model normals
void Renderer::_DrawTris(Program &program, GLuint vao, GLuint face_count, const mat4 &wm_transform, const mat4 &wm_normal_transform, const int textureID, const Material& uniform_mat)
{
	// Bind the models settings
	glUseProgram(program.program);
	_passLights(program);
	glBindVertexArray(vao);
	if(textureID != -1){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(program.find("ourTexture"), 0);
	}

	GLfloat full_transform_array[16];
	toFloatArray(full_transform_array, wm_transform);
	glUniformMatrix4fv(program.find("world_transform"), 1, GL_FALSE, full_transform_array);

	GLfloat proj_array[16];
	toFloatArray(proj_array, mat_project * mat_transform_inverse);
	glUniformMatrix4fv(program.find("camera_transform"), 1, GL_FALSE, proj_array);

	GLfloat normal_trans_array[16];
	toFloatArray(normal_trans_array, wm_normal_transform);
	glUniformMatrix4fv(program.find("normal_transform"), 1, GL_FALSE, normal_trans_array);

	GLfloat camera_array[3] = {camera_position.x, camera_position.y, camera_position.z};
	glUniform3fv(program.find("camera_position"), 1, camera_array);
	// Draw
	glDrawArrays(GL_TRIANGLES, 0, face_count * 3);
	glBindVertexArray(0);
}

/// @brief Render a mesh with the current selected program
/// @param program Program class
/// @param vao vao of the vertices
/// @param face_count amount of vertices/3
/// @param wm_transform world*model transform of the model
/// @param wm_normal_transform world*model transform of the model normals
void Renderer::DrawMesh(GLuint vao, GLuint face_count, const mat4 &wm_transform, const mat4 &wm_normal_transform, const int textureID ,const Material& uniform_mat)
{
	//send the designated texture program
	if(textureID != -1){
		_DrawTris(program_texture, vao, face_count, wm_transform, wm_normal_transform, textureID, uniform_mat);
	}
	else{
		_DrawTris(programs[current_program], vao, face_count, wm_transform, wm_normal_transform, textureID, uniform_mat);
	}
}

/// @brief Identical to DrawMesh, ut GL_LINE_STRIP, and a hardcoded program.
/// @param program Program class
/// @param vao vao of the vertices
/// @param face_count amount of vertices/3
/// @param wm_transform world*model transform of the model
/// @param wm_normal_transform world*model transform of the model normals
void Renderer::DrawWireframe(GLuint vao, GLuint face_count, const mat4 &wm_transform)
{

	glUseProgram(program_wireframe.program);
	GLfloat color_arr[3] = {0.8, 0.8, 0.8};
	glUniform3fv(program_wireframe.find("color"), 1, color_arr);

	// set the polygons to draw as lines
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.3f);
	glEnable(GL_LINE_SMOOTH);
	_DrawTris(program_wireframe, vao, face_count, wm_transform, mat4());

	glLineWidth(1.0f);
	// revert to default
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::DrawLines(GLuint lines_vao, GLuint lines_count, const mat4 &wm_transform, vec3 color)
{
	glUseProgram(program_wireframe.program);

	// Bind the models settings
	glBindVertexArray(lines_vao);

	GLfloat color_arr[3] = {color.x, color.y, color.z};
	glUniform3fv(program_wireframe.find("color"), 1, color_arr);

	GLfloat full_transform_array[16];
	toFloatArray(full_transform_array, wm_transform);
	glUniformMatrix4fv(program_wireframe.find("world_transform"), 1, GL_FALSE, full_transform_array);

	GLfloat proj_array[16];
	toFloatArray(proj_array, mat_project * mat_transform_inverse);
	glUniformMatrix4fv(program_wireframe.find("camera_transform"), 1, GL_FALSE, proj_array);

	// Draw
	glDrawArrays(GL_LINES, 0, lines_count);
	glBindVertexArray(0);

	glUseProgram(0);
}

// Camera
void Renderer::SetCameraTransformInverse(const mat4 &cTransform)
{
	mat_transform_inverse = cTransform;
}
void Renderer::SetProjection(const mat4 &projection)
{
	mat_project = projection;
}

void Renderer::setCameraMatrixes(const mat4 &cTransformInverse, const mat4 &Projection, const vec3 &camera_pos)
{
	SetCameraTransformInverse(cTransformInverse);
	SetProjection(Projection);
	camera_position = camera_pos;
}

void Renderer::setCameraMatrixes(Camera *camera)
{
	setCameraMatrixes(camera->getTransformInverse(), camera->getProjection(), camera->getCameraPosition());
}

void Renderer::Init()
{
	CreateBuffers(m_width, m_height);
}

/////////////////////////////////////////////////////
// OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[] = {
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1};
	const GLfloat tex[] = {
		0, 0,
		1, 0,
		0, 1,
		0, 1,
		1, 0,
		1, 1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	GLint vPosition = glGetAttribLocation(program, "vPosition");

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0,
						  0);

	GLint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
						  (GLvoid *)sizeof(vtc));
	glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}