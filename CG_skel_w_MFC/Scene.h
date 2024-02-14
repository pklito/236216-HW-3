#pragma once


#include "mat.h"
#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

class Model {
	protected:
		int data;	//int that lets you store some extra metadata
public:
	Model() : data(0) {}
	virtual ~Model() {}
	void virtual draw(Renderer* renderer) = 0;
	virtual void setShowNormals(bool change) = 0;
	virtual void setShowNormalsToVertices(bool change) = 0;
	virtual void setShowBox(bool change) = 0;
	virtual void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) = 0;
	virtual void rotate(GLfloat theta_angle, int axis) = 0;
	virtual void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale) = 0;

	virtual void applyWorldTransformation(const mat4& transformation) = 0;
	virtual void applyModelTransformation(const mat4& transformation) = 0;
	void setData(int dat) {data = dat;}
};


class Light {

};

class Camera {
	mat4 cTransform;
	mat4 cTransformInverse;
	mat4 projection;

	// Constants for perspective projection
	static constexpr float FOV = 60.0f; // Field of View in degrees
	static constexpr float NEAR_CLIPPING_PLANE = 0.1f;
	static constexpr float FAR_CLIPPING_PLANE = 100.0f;

public:
	void setTransformation(const mat4& transform);
	void setProjection(const mat4& perspective);
	void UpdateProjectionMatrix(float aspect_ratio);
	mat4 getProjection();
	mat4 getTransform();
	mat4 getTransformInverse();

	void draw(Renderer* renderer);
	vec3 getCameraPosition();
	void setInverseTransformation(const mat4& InvTransform);
	void applyWorldInverseTransformation(const mat4& InvMatrix);
	void applyScreenInverseTransformation(const mat4& InvMatrix);
	void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans, bool in_world);
	void rotate(GLfloat theta_angle, int mode, bool in_world);
	void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale, bool in_world);

	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);

};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

	bool world_control;
	bool moving_model;

public:
	Scene() : world_control(false), moving_model(true), activeModel(0), activeLight(0), activeCamera(0) {};
	Scene(Renderer *renderer) : m_renderer(renderer), world_control(false), moving_model(true), activeModel(0), activeLight(0), activeCamera(0) {};
	void loadOBJModel(string fileName);
	void addMeshModel(Model* model);
	void addCamera(Camera* camera);
	void draw();
	void drawDemo();

	void setShowNormalsForMeshModels(bool change);
	void setShowNormalsToVerticesForMeshModels(bool change);
	void setShowBoxForMeshModels(bool change);
	void translateObject(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans);
	void scaleObject(GLfloat scale);
	void rotateObject(GLfloat theta_angle, int axis);
	void cycleSelectedObject();
	void cycleActiveCamera();
	void removeSelectedObject();
	void removeSelectedCamera();
	Camera* getActiveCamera();

	void setWorldControl(bool ctrl);
	bool getWorldControl();

	int activeModel;
	int activeLight;
	int activeCamera;
};