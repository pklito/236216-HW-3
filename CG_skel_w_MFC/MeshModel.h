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

	bool show_vertex_normals;
	bool show_face_normals;
	bool show_box;

	vec3* normals;
	vec3 bounding_box[8];
	//add more attributes
	mat4 _world_transform;
	mat3 _normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer) override;

	void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) override;
	vec3 translatePoint(vec3 point, GLfloat x_trans, GLfloat y_trans, GLfloat z_trans);
	void rotate(GLfloat theta_angle, int mode) override;
	void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale) override;
	void calculateBoundingBox();
	void normalToFace();

	void setShowNormals(bool change) override;
	void setShowBox(bool change) override;
};

vec3 calculateNormal(vec3 first_point, vec3 second_point, vec3 third_point);