#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

using namespace std;
void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves

	m_renderer->SwapBuffers();
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}


void Camera::setTransformation(const mat4& transform) {

}
void Camera::setProjection(const mat4& perspective) {

}
mat4 Camera::getProjection() {
	return projection;
}
mat4 Camera::getTransform() {
	return cTransform;
}

mat4 Camera::getTransformInverse() {
	return cTransformInverse;
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up) {
	/* Create 3 directional vectors (Tut 3: slide 9)*/
	vec4 n = vec4(normalize(toVec3(eye - at)));	//I make it Vec3 because normalizing with W would be wrong
	vec4 u = vec4(cross(up, n), 0);
	vec4 v = vec4(cross(n, u), 0);

	mat4 rotate_inv = mat4(u, v, n, vec4(0, 0, 0, 1));

	// set the matrixes stored.
	cTransformInverse = rotate_inv * Translate(-eye);
	cTransform = Translate(eye) * transpose(rotate_inv);
}
void Camera::Ortho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	projection = mat4(2 / (right - left), 0, 0, -(left + right) / (right - left),
		0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
		0, 0, -2 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear),
		0, 0, 0, 1);
}

void Camera::Frustum(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {

}

void Camera::Perspective(const float fovy, const float aspect,
	const float zNear, const float zFar) {

}