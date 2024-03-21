#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel() {}
	GLuint vao;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_textures;
	//add more attributes
	mat4 _world_transform;
	mat3 _normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw();
	
};
