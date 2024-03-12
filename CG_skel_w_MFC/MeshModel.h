#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include "util.h"

using namespace std;

class MeshModel : public Model
{
protected:
	MeshModel() : vertex_normals_exist(false), show_vertex_normals(false), show_face_normals(false), fill_obj(false),special_color(0) {}
	vec3* vertex_positions;
	vec3* normals_to_vertices;
	int curr_color;
	int special_color;
	Material material;

	bool vertex_normals_exist;
	int face_count;

	bool show_vertex_normals;
	bool show_face_normals;
	bool show_box;
	bool fill_obj;

	vec3* normals;
	vec3 bounding_box[8];
	//add more attributes
	mat4 _world_transform;
	mat3 _normal_transform;

	void updateMaterial();

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw(Renderer* renderer) override;

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

	void setMaterial(const Material& new_mat){material = new_mat;}
	Material getMaterial(){return material;}
	void toggleSpecialMaterial() override;
	void resetToCenter();
	vec3 calculateBoundingBoxCenter();
};

typedef enum{
	PRIM_CUBE,
	PRIM_TETRAHEDRON
} PRIM_MODEL;
class PrimMeshModel : public MeshModel
{
	protected:
		void Cube();
		void Tetrahedron();
	public:
	PrimMeshModel(PRIM_MODEL model){
		vertex_normals_exist = false;
		show_vertex_normals = false;
		show_face_normals = false;
		switch(model){
			case PRIM_CUBE:
				Cube();
				break;
			case PRIM_TETRAHEDRON:
				Tetrahedron();
				break;
		}
		calculateBoundingBox();
	}

};

vec3 calculateNormal(vec3 first_point, vec3 second_point, vec3 third_point);