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

void Camera::UpdateProjectionMatrix(float aspect_ratio)
{
	// Assuming you have member variables like fov, near_clip, and far_clip
	Perspective(radians(FOV), aspect_ratio, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
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

void Scene::addCamera(Camera* camera){
	cameras.push_back(camera);
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
	if(world_control){
		models[activeModel]->applyWorldTransformation(Translate(x_trans,y_trans,z_trans));
	}
	else {
		//should be: models[activeModel]->applyModelTransformation(Translate(x_trans,y_trans,z_trans));
		models[activeModel]->translate(x_trans,y_trans,z_trans);
	}

}

void Scene::scaleObject(GLfloat scale)
{
	if(world_control){
		models[activeModel]->applyWorldTransformation(Scale(scale,scale,scale));
	}
	else{
		models[activeModel]->scale(scale,scale,scale);
	}
}
void Scene::rotateObject(GLfloat theta_angle, int axis)
{
	if(world_control){
		mat4 rotate_mat = RotateAxis(theta_angle,axis);
		models[activeModel]->applyWorldTransformation(rotate_mat);
	}
	else{
		models[activeModel]->rotate(theta_angle,axis);
	}
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	for(auto it = models.begin(); it != models.end(); it++){
		(*(it))->draw(m_renderer);
	}
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}

void Scene::cycleSelectedObject()
{
	activeModel = (activeModel+1) % models.size();
}

void Scene::cycleActiveCamera()
{
	activeCamera = (activeCamera+1) % cameras.size();
}

Camera* Scene::getActiveCamera()
{
	return cameras[activeCamera];
}

//---------------------
//   CAMERA
//---------------------
void Camera::setInverseTransformation(const mat4& InvTransform){
	cTransformInverse = InvTransform;
}
void Camera::applyWorldInverseTransformation(const mat4& InvMatrix){
	cTransformInverse = cTransformInverse * InvMatrix;
}

void Camera::applyScreenInverseTransformation(const mat4& InvMatrix){
	cTransformInverse =  InvMatrix * cTransformInverse;
}

void Camera::translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans, bool in_world)
{
	//Inverse of Translate
	if(in_world){
		applyWorldInverseTransformation(Translate(-x_trans,-y_trans,-z_trans));
	}
	else{
		applyScreenInverseTransformation(Translate(-x_trans,-y_trans,-z_trans));
	}
}

void Camera::rotate(GLfloat theta_angle, int axis, bool in_world)
{
	//Inverse of Rotate
	if(in_world){
		applyWorldInverseTransformation(RotateAxis(-theta_angle,axis));
	}
	else{
		applyScreenInverseTransformation(RotateAxis(-theta_angle,axis));
	}

}

void Camera::scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale, bool in_world)
{
	if(abs(x_scale)<0.01 || abs(y_scale)<0.01 || abs(z_scale)<0.01)
		return;

	//Inverse of Scale
	if(in_world){
		applyWorldInverseTransformation(Scale(1/x_scale,1/y_scale,1/z_scale));
	}
	else{
		applyWorldInverseTransformation(Scale(1/x_scale,1/y_scale,1/z_scale));
	}
}

void Camera::setTransformation(const mat4& transform) {
	cTransform = transform;
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
	std::cout << "n u v: " << n << u << v << std::endl;
	mat4 rotate_inv = mat4(u,v,n,vec4(0,0,0,1));
	std::cout << "Rot Matrix: " << rotate_inv << std::endl;
	std::cout << "Transform Matrix: " << Translate(-eye) << std::endl;
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

