#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected:
	MeshModel() : draw_wireframe(false), draw_vertex_normals(false),draw_bounding_box(false), draw_face_normals(false) {}
	void generateBuffers( int face_num, const GLfloat *vertices_array, const GLfloat *vertex_normals_array, const GLfloat *vertex_textures_array, const GLfloat *vertex_material_array);

	int face_num;
	GLuint vao;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_textures;

	GLuint vert_vao;
	GLuint vert_vbo;

	GLuint face_vao;
	GLuint face_vbo;

	GLuint box_vao;
	GLuint box_vbo;

	// add more attributes
	mat4 _world_transform;
	mat4 _model_transform;
	mat4 _world_normal_transform;
	mat4 _model_normal_transform;

	bool draw_wireframe;
	bool draw_vertex_normals;
	bool draw_face_normals;
	bool draw_bounding_box;

	void applyWorldTransformation(const mat4 &transformation);
	void applyModelTransformation(const mat4 &transformation);
	void applyWorldNormalTransformation(const mat4 &transformation_inv);
	void applyModelNormalTransformation(const mat4 &transformation_inv);

public:
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer *renderer) override;

	// TODO UNIMPLEMENTED:
	mat4 getFullTransformation();
	void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans, bool isWorld = false) override;
	void rotate(GLfloat theta_angle, int mode, bool isWorld = false) override;
	void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale, bool isWorld = false) override;

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
};

typedef enum
{
	PRIM_CUBE,
	PRIM_TETRAHEDRON
} PRIM_MODEL;
class PrimMeshModel : public MeshModel
{
protected:
	void Cube();
	void Tetrahedron();

public:
	PrimMeshModel(PRIM_MODEL model)
	{
		switch (model)
		{
		case PRIM_CUBE:
			Cube();
			break;
		case PRIM_TETRAHEDRON:
			Tetrahedron();
			break;
		}
	}
};
