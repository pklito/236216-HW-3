#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "GL/freeglut.h"
#include "InitShader.h"

using namespace std;

struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}
};

vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

MeshModel::MeshModel(string fileName)
{
	loadFile(fileName);
}

MeshModel::~MeshModel(void)
{
	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &vbo_textures);
	glDeleteBuffers(1, &vbo_normals);
	glDeleteVertexArrays(1, &vao);
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> vertex_normals;
	vector<vec3> vertex_textures;
	// while not end of file
	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;

		// based on the type parse data
		if (lineType == "v")	//BUG FIXED
			vertices.push_back(vec3fFromStream(issLine));
		else if (lineType == "f")	//BUG FIXED
			faces.push_back(issLine);
		else if (lineType == "vn") {
			//normal to vertices
			vertex_normals.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "vt") {
			vertex_textures.push_back(vec3fFromStream(issLine));
			//texture
		}
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout << "Found unknown line Type \"" << lineType << "\"";
		}
	}
	
	//Three vertices per face, three coordinates per vertex
	
	face_num = faces.size();
	static GLfloat* vertices_array = new GLfloat[face_num*3 * 3];
	static GLfloat* vertex_normals_array = new GLfloat[face_num*3*3];
	static GLfloat* vertex_textures_array = new GLfloat[face_num*3*3];
	// * Convert buffers to float arrays to send to the GPU *
	int k = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int coord = 0; coord < 3; coord ++){ //x, y, z
				vertices_array[k + coord] = vertices[it->v[i] - 1][coord];
				vertex_normals_array[k + coord] = vertex_normals[it->vn[i] - 1][coord];
				//vertex_textures_array[k + coord] = vertex_textures[it->vt[i] - 1][coord]; 	//Take the face indexes from the vertix array BUG FIXED
			}
			// iterate to next face
			k+=3;
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint vbos[3] = {0,0,0};
	glGenBuffers(3, vbos);
	vbo_vertices = vbos[0];
	vbo_normals = vbos[1];
	vbo_textures = vbos[2];


	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, face_num*sizeof(float)*3*3,
		vertices_array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, face_num*sizeof(float)*3*3,
		vertex_normals_array, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1); 

	glBindBuffer(GL_ARRAY_BUFFER, vbo_textures);
	glBufferData(GL_ARRAY_BUFFER, face_num*sizeof(float)*3*3,
		vertex_textures_array, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	delete[] vertices_array;
	delete[] vertex_normals_array;
	delete[] vertex_textures_array;
}

void MeshModel::draw(Renderer* renderer)
{
	renderer->DrawMesh(vao, face_num, _world_transform * _model_transform);	//TODO: calculate this transform on change
}

// TODO: Implement this function
mat4 MeshModel::getWorldTransformation() {
	// Placeholder implementation
	return mat4(); // Return identity matrix for now
}

// TODO: Implement this function
void MeshModel::translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::rotate(GLfloat theta_angle, int mode) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::calculateBoundingBox() {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::normalToFace() {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::CalculateVertexNormals() {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::applyWorldTransformation(const mat4& transformation) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::applyModelTransformation(const mat4& transformation) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::setShowNormals(bool change) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::setShowNormalsToVertices(bool change) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::setShowBox(bool change) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::setFillObj(bool fill) {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::changeColor() {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::toggleSpecialMaterial() {
	// Placeholder implementation
}

// TODO: Implement this function
void MeshModel::resetToCenter() {
	// Placeholder implementation
}

// TODO: Implement this function
vec3 MeshModel::calculateBoundingBoxCenter() {
	// Placeholder implementation
	return vec3(); // Return default vec3 for now
}


//------------
// PRIM
//------------


void PrimMeshModel::Cube()
{
	const vec3 cube_points[] = {vec3(-0.5f, -0.5f, -0.5f),vec3(0.5f, -0.5f, -0.5f),vec3(0.5f, 0.5f, -0.5f),vec3(-0.5f, 0.5f, -0.5f),vec3(-0.5f, -0.5f, 0.5f),vec3(0.5f, -0.5f, 0.5f),vec3(0.5f, 0.5f, 0.5f),vec3(-0.5f, 0.5f, 0.5f)};
	const int face_indices[] = {
        0, 1, 2,
        2, 3, 0,
        1, 5, 6,
        6, 2, 1,
        5, 4, 7,
        7, 6, 5,
        4, 0, 3,
        3, 7, 4,
        3, 2, 6,
        6, 7, 3,
        0, 4, 5,
        5, 1, 0
    };

	// Hardcoded cube vertices
	face_num = 12;
	
	static GLfloat vertices_array[12 * 3 * 3];
	for(int i = 0; i < 3*face_num; i++){
		for(int coord = 0; coord < 3; coord++){
		vertices_array[3*i + coord] = cube_points[face_indices[i]][coord];
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo_vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, face_num*sizeof(float)*3*3,
		vertices_array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}


void PrimMeshModel::Tetrahedron()
{
	#define base3 0, 0, -1
	#define base2 -0.866, 0, 0.5
	#define base1 0.866, 0, 0.5
	#define top 0, 1, 0
	face_num = 4;
	static GLfloat vertices_array[3*4*3] = {base3, base2, base1, top, base3, base1, top, base1, base2, top, base2, base3};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo_vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, face_num*sizeof(float)*3*3,
		vertices_array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}