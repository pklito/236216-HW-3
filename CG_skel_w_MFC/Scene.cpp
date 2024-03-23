#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include "InitShader.h"
#include "GL\freeglut.h"

using namespace std;
void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	GLuint program = InitShader( "minimal_vshader.glsl", 
					  "minimal_fshader.glsl" );

	glUseProgram(program);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);

	for(auto it = models.begin(); it != models.end(); it++){
		(*(it))->draw(m_renderer);
	}

	glFlush();
	glutSwapBuffers();
}

void Scene::drawDemo()
{
	std::cout << "out " << std::endl;
	const int pnum = 3;
	static const GLfloat points[pnum][3] = {
		{-0.1, -0.1f, 0.0f},
		{0.1f, -0.1f, 0.0f},
		{0.0f,  0.1f, 0.0f}
	};

	GLuint program = InitShader( "minimal_vshader.glsl", 
					  "minimal_fshader.glsl" );

	glUseProgram(program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points),
		points, GL_STATIC_DRAW);



	
	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, pnum);
	glFlush();
	glutSwapBuffers();
	

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


