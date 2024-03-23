#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include "InitShader.h"
#include "GL\freeglut.h"

using namespace std;
void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	GLuint program = InitShader( "minimal_vshader.glsl", 
					  "minimal_fshader.glsl" );

	glUseProgram(program);
	
	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	std::cout<<"started" << std::endl;
	for(auto it = models.begin(); it != models.end(); it++){
		(*(it))->draw(m_renderer);
	}

	glFlush();
	glutSwapBuffers();
}

void Scene::drawDemo()
{
	std::cout << "out " << std::endl;
	const int pnum = 3;
	static const GLfloat points[pnum][3] = {
		{-0.1, -0.1f, 0.0f},
		{0.1f, -0.1f, 0.0f},
		{0.0f,  0.1f, 0.0f}
	};

	GLuint program = InitShader( "minimal_vshader.glsl", 
					  "minimal_fshader.glsl" );

	glUseProgram(program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points),
		points, GL_STATIC_DRAW);



	
	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, pnum);
	glFlush();
	glutSwapBuffers();
	

}
