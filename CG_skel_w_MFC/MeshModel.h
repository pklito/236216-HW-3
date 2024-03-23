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
	int face_num;
	GLuint vao;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_textures;
	//add more attributes
	mat4 _world_transform;
	mat4 _model_transform;
	mat4 _world_normal_transform;
	mat4 _model_normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer) override;

	// TODO UNIMPLEMENTED: 
	mat4 getWorldTransformation();
	void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) override;
	void rotate(GLfloat theta_angle, int mode) override;
	void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale) override;

	void calculateBoundingBox();
	void normalToFace();
	void CalculateVertexNormals();

	void applyWorldTransformation(const mat4& transformation) override;
	void applyModelTransformation(const mat4& transformation) override;

	void setShowNormals(bool change) override;
	void setShowNormalsToVertices(bool change) override;
	void setShowBox(bool change) override;
	void setFillObj(bool fill);

	void changeColor();

	/*
	void setMaterial(const Material& new_mat){material = new_mat;}
	Material getMaterial(){return material;}
	*/
	void toggleSpecialMaterial() override;
	void resetToCenter();
	vec3 calculateBoundingBoxCenter();


	
};
