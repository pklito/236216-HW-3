#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

using namespace std;

float radians(float degrees) {
	return degrees * (M_PI / 180.0f);
}

void Scene::setShowNormalsToVerticesForMeshModels(bool change) {
	for (Model* model : models) {
		// Check if the model is of type MeshModel
		MeshModel* meshModel = dynamic_cast<MeshModel*>(model);
		if (meshModel != nullptr) {
			// It's a MeshModel, call setShowNormals
			meshModel->setShowNormalsToVertices(change);
		}
		// You can handle other types of models here if needed
	}
}

void Camera::Perspective(float fovy, float aspect, float zNear, float zFar) {
	Frustum(-fovy*aspect*zNear,fovy*aspect*zNear,-fovy*zNear,fovy*zNear,zNear,zFar);
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::addMeshModel(Model* model)
{
	models.push_back(model);
}

void Scene::addLightSource(Light* light)
{
	lights.push_back(light);
}

void Scene::addFog(Fog* fog)
{
	fogs.push_back(fog);
}

void Scene::changeShadingMethod()
{
	m_renderer->changeShadingMethod();
}

void Scene::addCamera(Camera* camera){
	cameras.push_back(camera);
	if (cameras.size() == 1) {
		vec3 cameraPosition = camera->getCameraPosition();
		m_renderer->setCameraPos(cameraPosition);
	}
	
}

void Scene::setWorldControl(bool ctrl){
	world_control = ctrl;
}
bool Scene::getWorldControl(){
	return world_control;
}

// Iterate over the models and call setShowNormals for MeshModels
void Scene::setShowNormalsForMeshModels(bool change) {
	for (Model* model : models) {
		// Check if the model is of type MeshModel
		MeshModel* meshModel = dynamic_cast<MeshModel*>(model);
		if (meshModel != nullptr) {
			// It's a MeshModel, call setShowNormals
			meshModel->setShowNormals(change);
		}
		// You can handle other types of models here if needed
	}
}

void Scene::setShowBoxForMeshModels(bool change) {
	for (Model* model : models) {
		// Check if the model is of type MeshModel
		MeshModel* meshModel = dynamic_cast<MeshModel*>(model);
		if (meshModel != nullptr) {
			// It's a MeshModel, call setShowBox
			meshModel->setShowBox(change);
		}
		// You can handle other types of models here if needed
	}
}

void Scene::translateObject(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans)
{
	if(!moving_model){
		if(lights.size() >= 1){
			lights[activeLight]->translate(x_trans,y_trans,z_trans);
		}
		return;
	}

	if(world_control){
		if (models.size() >= 1) {
			models[activeModel]->applyWorldTransformation(Translate(x_trans, y_trans, z_trans));
		}
	}
	else {
		//should be: models[activeModel]->applyModelTransformation(Translate(x_trans,y_trans,z_trans));
		if (models.size() >= 1) {
			models[activeModel]->translate(x_trans, y_trans, z_trans);
		}
	}

}

void Scene::returnModelToCenter()
{
	if (models.size() >= 1) {
		models[activeModel]->resetToCenter();
	}
}

void Scene::scaleObject(GLfloat scale)
{
	if(!moving_model){
		if(lights.size() >= 1){
			lights[activeLight]->translate(scale,scale,scale);
		}
		return;
	}
	
	if(world_control){
		if (models.size() >= 1) {
			models[activeModel]->applyWorldTransformation(Scale(scale, scale, scale));
		}
	}
	else{
		if (models.size() >= 1) {
			models[activeModel]->scale(scale, scale, scale);
		}
	}
}
void Scene::rotateObject(GLfloat theta_angle, int axis)
{
	if(!moving_model){
		if(lights.size() >= 1){
			lights[activeLight]->rotate(theta_angle, axis);
		}
		return;
	}
	if(world_control){
		if (models.size() >= 1) {
			mat4 rotate_mat = RotateAxis(theta_angle, axis);
			models[activeModel]->applyWorldTransformation(rotate_mat);
		}
	}
	else{
		if (models.size() >= 1) {
			models[activeModel]->rotate(theta_angle, axis);
		}
	}
}

void Scene::changeCurrsColor()
{
	models[activeModel]->changeColor();
}

void Scene::draw()
{
	int i = 0;
	for(auto it = cameras.begin(); it != cameras.end(); it++){
		if(i!=activeCamera){
			(*it)->draw(m_renderer);
		}
		i++;
	}

	for(auto& light : lights){
		m_renderer->DrawLightSymbol(light);
	}

	
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	for(auto it = models.begin(); it != models.end(); it++){
		(*(it))->draw(m_renderer);
	}

	if(world_control){
		m_renderer->DrawSymbol(vec3(0,0,0),mat4(),SYM_PLUS,1,vec3(0.1,0.5,0.9));
	}
	else{
		if (models.size() >= 1) {
			m_renderer->DrawSymbol(vec3(0, 0, 0), models[activeModel]->getWorldTransformation(), SYM_PLUS, 1, vec3(0.1, 0.5, 0.9));
		}
	}

}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}

void Scene::cycleSelectedObject()
{
	if (models.size() >= 1) {
		if (activeModel == models.size()) {
			activeModel = (activeModel + 1) % models.size();
			models[activeModel]->setData(1);
			return;
		}
		models[activeModel]->setData(0);
		activeModel = (activeModel + 1) % models.size();
		models[activeModel]->setData(1);
	}
}

void Scene::cycleActiveCamera()
{
	activeCamera = (activeCamera+1) % cameras.size();
	vec3 cameraPosition = getActiveCamera()->getCameraPosition();
	m_renderer->setCameraPos(cameraPosition);
}

void Scene::setFillObj(bool fill)
{
	fillCurrObj = fill;
	if (models.size() >= 1) {
		models[activeModel]->setFillObj(fillCurrObj);
	}
}

bool Scene::getFillObj()
{
	return fillCurrObj;
}

void Scene::removeSelectedObject(){
	if(models.size() < 1)
		return;

	models.erase(models.begin()+activeModel);
	cycleSelectedObject();
}

void Scene::removeSelectedCamera(){
	if(cameras.size() <= 1)
		return;
	cameras.erase(cameras.begin()+activeCamera);
	cycleActiveCamera();
}

Camera* Scene::getActiveCamera()
{
	return cameras[activeCamera];
}

void Scene::rotateCameraToSelectedObject(){
	vec4 model_center = models[activeModel]->getWorldTransformation() * vec4(0, 0, 0, 1);
	vec4 camera_location = cameras[activeCamera]->getCameraPosition();
	cameras[activeCamera]->LookAt(camera_location, model_center, vec3(0, 1, 0));

}


//---------------------
//   CAMERA
//---------------------


void Camera::draw(Renderer* renderer){
	renderer->DrawSymbol(getCameraPosition(),mat4(),SYM_SQUARE, 1);
}
vec3 Camera::getCameraPosition(){
	vec4 base = vec4(1,2,3,1);
	vec4 point = cTransform * vec4(0,0,0,1);
	return toVec3(point);
}

void Camera::_applyWorldTransformInverse(const mat4& InvMatrix){
	cTransformInverse = cTransformInverse * InvMatrix;
}

void Camera::_applyScreenTransformInverse(const mat4& InvMatrix){
	cTransformInverse =  InvMatrix * cTransformInverse;
}

void Camera::_applyWorldTransform(const mat4& matrix){
	cTransform = matrix * cTransform;
}

void Camera::_applyScreenTransform(const mat4& matrix){
	cTransform = cTransform * matrix;
}

void Camera::translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans, bool in_world)
{
	//Inverse of Translate
	if(in_world){
		_applyWorldTransformInverse(Translate(-x_trans,-y_trans,-z_trans));
		_applyWorldTransform(Translate(x_trans,y_trans,z_trans));
	}
	else{
		_applyScreenTransformInverse(Translate(-x_trans,-y_trans,-z_trans));
		_applyScreenTransform(Translate(x_trans,y_trans,z_trans));
	}
}

void Camera::rotate(GLfloat theta_angle, int axis, bool in_world)
{
	//Inverse of Rotate
	if(in_world){
		_applyWorldTransformInverse(RotateAxis(-theta_angle,axis));
		_applyWorldTransform(RotateAxis(theta_angle,axis));
	}
	else{
		_applyScreenTransformInverse(RotateAxis(-theta_angle,axis));
		_applyScreenTransform(RotateAxis(theta_angle,axis));
	}

}

void Camera::scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale, bool in_world)
{
	if(abs(x_scale)<0.01 || abs(y_scale)<0.01 || abs(z_scale)<0.01)
		return;

	//Inverse of Scale
	if(in_world){
		_applyWorldTransformInverse(Scale(1/x_scale,1/y_scale,1/z_scale));
		_applyWorldTransform(Scale(x_scale,y_scale,z_scale));
	}
	else{
		_applyScreenTransformInverse(Scale(1/x_scale,1/y_scale,1/z_scale));
		_applyScreenTransform(Scale(x_scale,y_scale,z_scale));
	}
}

void Camera::setProjection(const mat4& perspective) {
	projection = perspective;
}

mat4 Camera::getProjection(){
	return projection;
}
mat4 Camera::getTransform(){
	return cTransform;
}

mat4 Camera::getTransformInverse(){
	return cTransformInverse;
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up ){
	/* Create 3 directional vectors (Tut 3: slide 9)*/
	vec4 n = vec4(normalize(toVec3(eye - at)),0);	//I make it Vec3 because normalizing with W would be wrong
	vec4 u = vec4(cross(up, n),0);
	vec4 v = vec4(cross(n, u),0);
	mat4 rotate_inv = mat4(u,v,n,vec4(0,0,0,1));
	// set the matrixes stored.
	cTransformInverse = rotate_inv * Translate(-eye);
	cTransform = Translate(eye) * transpose(rotate_inv);
}

void Camera::Ortho( const float left, const float right, const float bottom, const float top, const float zNear, const float zFar ){
	projection = mat4(vec4(2/(right-left),   0, 				0, 		-(left+right)/(right-left)),
					  vec4(0,				2/(top-bottom), 0, 		-(top+bottom)/(top-bottom)),
					  vec4(0,				0,	-2/(zFar-zNear),	-(zFar + zNear)/(zFar - zNear)),
					  vec4(0,				0,				0,		1));
}


void Camera::Frustum(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
    projection = mat4(
       vec4( (2*zNear)/(right-left),  0,                      (right+left)/(right-left),      0),
        vec4(0,                        (2*zNear)/(top-bottom), (top+bottom)/(top-bottom),      0),
        vec4(0,                        0,                      -(zFar+zNear)/(zFar-zNear),    -2*zFar*zNear/(zFar-zNear)),
        vec4(0,                        0,                      -1,                            0)
    );
}

