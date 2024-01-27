#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

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
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
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
		if (lineType == "?") /*BUG*/
			vertices.push_back(vec3fFromStream(issLine));
		else if (lineType == "?") /*BUG*/
			faces.push_back(issLine);
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout<< "Found unknown line Type \"" << lineType << "\"";
		}
	}
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}

	vertex_positions = new vec3[7]; /*BUG*/
	// iterate through all stored faces and create triangles
	int k=0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			vertex_positions[k++] = vec3(); /*BUG*/
		}
	}
}



void MeshModel::draw()
{
	
}

void MeshModel::translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans) 
{
	int i = 0;
	while (i != sizeof(vertex_positions) / sizeof(vec3)) {
		vec3 current_vertex = vertex_positions[i];

		current_vertex[0] += x_trans;
		current_vertex[1] += y_trans;
		current_vertex[2] += z_trans;
		i++;
	}
}

void MeshModel::scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale)
{
	if (x_scale == 0 || y_scale == 0 || z_scale == 0) {
		return;
	}

	int i = 0;
	while (i != sizeof(vertex_positions) / sizeof(vec3)) {
		vec3 current_vertex = vertex_positions[i];

		current_vertex[0] *= x_scale;
		current_vertex[1] *= y_scale;
		current_vertex[2] *= z_scale;
		i++;
	}
}

void MeshModel::rotate(GLfloat theta_angle)
{
	int i = 0;
	while (i != sizeof(vertex_positions) / sizeof(vec3)) {
		vec3 current_vertex = vertex_positions[i];
		mat4 rotation_matrixX = RotateX(theta_angle);
		vec3 rotated_point = vec3(rotation_matrixX * vec4(current_vertex, 1.0f));
		vertex_positions[i] = rotated_point;
		i++;
	}
	
}

vec3 calculateNormal(vec3 first_point, vec3 second_point, vec3 third_point)
{
	vec3 a = third_point - first_point;
	vec3 b = second_point - first_point;

	vec3 c = cross(a, b);
	
	return normalize(c);
}

void MeshModel::normalToFace() 
{
	int i = 0;
	while (i != sizeof(vertex_positions) / sizeof(vec3)) {
		vec3 curr_normal = calculateNormal(vertex_positions[i], vertex_positions[i+1], vertex_positions[i+2]);
		normals[2*i/3] = curr_normal;
		normals[(2 * i / 3) + 1] = (vertex_positions[i] + vertex_positions[i + 1] + vertex_positions[i + 2]) / 3;
		i += 3;
	}
	draw();

}