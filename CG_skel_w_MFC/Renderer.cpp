#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include "scene.h"
#include "Texture.h"

#define INDEX(width, x, y, c) ((x) + (y) * (width)) * 3 + c

Renderer::Renderer() : Renderer(512, 512, "vshader.glsl", "fshader.glsl")
{
}

Renderer::Renderer(int width, int height, const char* vshader, const char* fshader) : m_width(width), m_height(height), programs(), current_program(0)
{
	InitOpenGLRendering();

	glGenTextures(1, &gScreenTex);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);

	// Set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load and generate the texture (if needed)
	int img_width, img_height, nrChannels; // Use different variable names
	unsigned char* data = stbi_load("meshes/shirt4.jpg", &img_width, &img_height, &nrChannels, 0); // Update variable names here
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		std::cout << "Texture loaded successfully." << std::endl; // Add this line for debug
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	CreateBuffers(width, height);
	CreateProgram(vshader, fshader);
	program_wireframe = Program("lines_vshader.glsl", "lines_fshader.glsl", "world_transform", "camera_transform", "normal_transform", "texture");
}

Renderer::~Renderer(void)
{
	for (Program p : programs)
	{
		p.Delete();
	}
}

void Renderer::StartDraw()
{
	glUseProgram(programs[current_program].program);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);
}

void Renderer::EndDraw()
{
	glFlush();
	glutSwapBuffers();
	glUseProgram(0);
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); // Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
}

void Renderer::CreateProgram(const char* vshader, const char* fshader)
{
	programs.push_back(Program(vshader, fshader, "world_transform", "camera_transform", "normal_transform", "texture"));
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

/// @brief Render a mesh with a certain program
/// @param program Program class
/// @param vao vao of the vertices
/// @param face_count amount of vertices/3
/// @param wm_transform world*model transform of the model
/// @param wm_normal_transform world*model transform of the model normals
void Renderer::_DrawTris(Program& program, GLuint vao, GLuint face_count, const mat4& wm_transform, const mat4& wm_normal_transform)
{
	texture_unit_index = program.program;

	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error in _DrawTris: " << error << std::endl;
	}
	// Bind the models settings
	glUseProgram(program.program);
	std::cout << "current program in _drawTris is: " << program.program << std::endl;
	glBindVertexArray(vao);

	GLfloat full_transform_array[16];
	toFloatArray(full_transform_array, wm_transform);
	glUniformMatrix4fv(program.find("world_transform"), 1, GL_FALSE, full_transform_array);

	GLfloat proj_array[16];
	toFloatArray(proj_array, mat_project * mat_transform_inverse);
	glUniformMatrix4fv(program.find("camera_transform"), 1, GL_FALSE, proj_array);

	GLfloat normal_trans_array[16];
	toFloatArray(normal_trans_array, wm_normal_transform);
	glUniformMatrix4fv(program.find("normal_transform"), 1, GL_FALSE, normal_trans_array);

	// Bind the texture to the specified texture unit index
	glActiveTexture(GL_TEXTURE0 + texture_unit_index);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	std::cout << "Texture bound successfully." << std::endl;

	// Set the uniform sampler2D to the texture unit index
	glUniform1i(program.find("texture"), texture_unit_index);

	glBindVertexArray(vao);
	// Set up vertex attribute pointer for texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, face_count * 3);

	// Unbind the vertex array object
	glBindVertexArray(0);
}



/// @brief Render a mesh with the current selected program
/// @param program Program class
/// @param vao vao of the vertices
/// @param face_count amount of vertices/3
/// @param wm_transform world*model transform of the model
/// @param wm_normal_transform world*model transform of the model normals
void Renderer::DrawMesh(GLuint vao, GLuint face_count, const mat4& wm_transform, const mat4& wm_normal_transform)
{

	_DrawTris(programs[current_program], vao, face_count, wm_transform, wm_normal_transform);
	std::cout << "current program in drawMesh is: " << programs[current_program].program << std::endl;
}

/// @brief Identical to DrawMesh, ut GL_LINE_STRIP, and a hardcoded program.
/// @param program Program class
/// @param vao vao of the vertices
/// @param face_count amount of vertices/3
/// @param wm_transform world*model transform of the model
/// @param wm_normal_transform world*model transform of the model normals
void Renderer::DrawWireframe(GLuint vao, GLuint face_count, const mat4& wm_transform)
{

	glUseProgram(program_wireframe.program);
	GLfloat color_arr[3] = { 0.8, 0.8, 0.8 };
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

void Renderer::DrawLines(GLuint lines_vao, GLuint lines_count, const mat4& wm_transform, vec3 color)
{
	glUseProgram(program_wireframe.program);

	// Bind the models settings
	glBindVertexArray(lines_vao);

	GLfloat color_arr[3] = { color.x, color.y, color.z };
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
void Renderer::SetCameraTransformInverse(const mat4& cTransform)
{
	mat_transform_inverse = cTransform;
}
void Renderer::SetProjection(const mat4& projection)
{
	mat_project = projection;
}

void Renderer::setCameraMatrixes(const mat4& cTransformInverse, const mat4& Projection)
{
	SetCameraTransformInverse(cTransformInverse);
	SetProjection(Projection);
}

void Renderer::setCameraMatrixes(Camera* camera)
{
	setCameraMatrixes(camera->getTransformInverse(), camera->getProjection());
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
		1, 1 };
	const GLfloat tex[] = {
		0, 0,
		1, 0,
		0, 1,
		0, 1,
		1, 0,
		1, 1 };
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
		(GLvoid*)sizeof(vtc));
	glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	while ((a = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (after glProgramUniform1i for texture): " << a << std::endl;
	}
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