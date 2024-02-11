#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected:
	MeshModel() {}
	vec3* vertex_positions;
	int vertex_count;
	int face_count;
	vec3* normals;
	vec3 bounding_box[8];
	//add more attributes
	mat4 _world_transform;
	mat3 _normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer);

	void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans);
	void rotate(GLfloat theta_angle);
	void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale);
	void calculateBoundingBox();
	void normalToFace();


};

vec3 calculateNormal(vec3 first_point, vec3 second_point, vec3 third_point);