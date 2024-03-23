#pragma once

#include "mat.h"
#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "util.h"
#include "Fog.h"
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
	virtual void setFillObj(bool fill) = 0;
	virtual void setShowBox(bool change) = 0;
	virtual void translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) = 0;
	virtual void rotate(GLfloat theta_angle, int axis) = 0;
	virtual void scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale) = 0;
	virtual mat4 getWorldTransformation() = 0;
	virtual void applyWorldTransformation(const mat4& transformation) = 0;
	virtual void applyModelTransformation(const mat4& transformation) = 0;
	virtual void toggleSpecialMaterial() = 0;
	void setData(int dat) {data = dat;}
	virtual void resetToCenter() = 0;
	virtual void changeColor() = 0;
};

class Camera {
	mat4 cTransform;
	mat4 cTransformInverse;
	mat4 projection;
	
	void _applyWorldTransformInverse(const mat4& InvMatrix);
	void _applyScreenTransformInverse(const mat4& InvMatrix);

	void _applyWorldTransform(const mat4& Matrix);
	void _applyScreenTransform(const mat4& Matrix);

public:
	void setProjection(const mat4& perspective);
	mat4 getProjection();
	mat4 getTransform();
	mat4 getTransformInverse();

	void draw(Renderer* renderer);
	vec3 getCameraPosition();
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
	Fog fog;
	vector<Camera*> cameras;
	Renderer *m_renderer;

	bool world_control;
	bool moving_model;
	bool fillCurrObj;

public:
	//Scene() : world_control(false), moving_model(true), activeModel(0), activeLight(0), activeCamera(0), fillCurrObj(false), {std::cout << "NO RENDERER PROVIDED!" << std::endl;};
	Scene(Renderer* renderer) : m_renderer(renderer), world_control(false), moving_model(true), activeModel(0), activeLight(0), activeCamera(0), fillCurrObj(false), fog() { /*m_renderer->setLights(&lights); m_renderer->setFog(&fogs); */};
	void loadOBJModel(string fileName);
	void draw();
	void drawDemo();

	
	bool getMovingModel() {return moving_model;}
	void setMovingModel(bool moving) {moving_model = moving;}
	bool toggleMovingModel() {moving_model = !moving_model; return moving_model;}

	// TODO UNIMPLEMENTED:
	void addMeshModel(Model* model);
	void addCamera(Camera* camera);
	void addLightSource(Light* light);
	void addFog(Fog* fog);
	
	void setShowNormalsForMeshModels(bool change);
	void setShowNormalsToVerticesForMeshModels(bool change);
	void setShowBoxForMeshModels(bool change);
	void translateObject(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans);
	void returnModelToCenter();
	void scaleObject(GLfloat scale);
	void rotateObject(GLfloat theta_angle, int axis);
	void cycleSelectedObject();
	void cycleActiveCamera();
	void removeSelectedObject();
	void removeSelectedCamera();
	void removeSelectedLight();
	Camera* getActiveCamera();
	void rotateCameraToSelectedObject();

	void setWorldControl(bool ctrl);
	bool getWorldControl();

	void setFillObj(bool fill);
	bool getFillObj();
	void changeCurrsColor();
	void changeShadingMethod();
	void changeCurrsMaterial();

	Material getSelectedMaterial();
	void setSelectedMaterial(const Material& mat);

	Light* getSelectedLight();
	Fog* getSelectedFog() {return &fog;};
	int activeModel;
	int activeLight;
	int activeCamera;

};