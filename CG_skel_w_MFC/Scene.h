#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

class Model {
public:
	virtual ~Model() {}
	void virtual draw(Renderer* renderer) = 0;
	virtual void setShowNormals(bool change);
	virtual void setShowBox(bool change) = 0;
	virtual void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) = 0;
	virtual void rotate(GLfloat theta_angle, int mode) = 0;
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
	int num_of_models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

public:
	Scene() {};
	Scene(Renderer *renderer) : m_renderer(renderer) {};
	void loadOBJModel(string fileName);
	void addMeshModel(Model* model);
	void draw();
	void drawDemo();

	void setShowNormalsForMeshModels(bool change);
	void setShowBoxForMeshModels(bool change);
	void translateObjects(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans);
	void rescaleModels(GLfloat scale);
	void rotateModels(GLfloat theta_angle, int mode);

	int activeModel;
	int activeLight;
	int activeCamera;
};