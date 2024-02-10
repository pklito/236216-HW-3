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
		for (int i = 0; i < 4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream& aStream)
	{
		for (int i = 0; i < 4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for (int i = 0; i < 3; i++)
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

vec3 vec3fFromStream(std::istream& aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream& aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

MeshModel::MeshModel(string fileName)
{
	vertex_count = 0;
	loadFile(fileName);
}

MeshModel::~MeshModel(void)
{
	delete[] vertex_positions;
	delete[] normals;
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
		if (lineType == "v")	//BUG FIXED
			vertices.push_back(vec3fFromStream(issLine));
		else if (lineType == "f")	//BUG FIXED
			faces.push_back(issLine);
		else if (lineType == "vn") {
			//normal
		}
		else if (lineType == "vt") {
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
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}
	face_count = faces.size();
	vertex_positions = new vec3[3 * faces.size()]; //In our project every face is a triangle. BUG FIXED
	normals = new vec3[3 * faces.size()];
	
	// iterate through all stored faces and create triangles
	int k = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			vertex_positions[k++] = vertices[it->v[i] - 1]; 	//Take the face indexes from the vertix array BUG FIXED
			vertex_count++;
		}
	}
	normalToFace();
	calculateBoundingBox();
	std::cout << "here is okay in loadFile after normalToFace" << std::endl;
}



void MeshModel::draw(Renderer* renderer)
{
	std::vector<vec3> vec(vertex_positions, vertex_positions + (3 * face_count));
	std::vector<vec3> norm(normals, normals + (3 * face_count));
	renderer->DrawTriangles(&vec, &norm);
	renderer->DrawBoundingBox(bounding_box);
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
		vec4 rotated_point = vec4(rotation_matrixX * vec4(current_vertex, 1.0f));
		vertex_positions[i] = vec3(rotated_point.x, rotated_point.y, rotated_point.z);
		i++;
	}

}

vec3 calculateNormal(vec3 first_point, vec3 second_point, vec3 third_point)
{
	vec3 normal_sized_first_point = vec3(100.0f * first_point.x, 100.0f * first_point.y, 100.0f * first_point.z);
	vec3 normal_sized_second_point = vec3(100.0f * second_point.x, 100.0f * second_point.y, 100.0f * second_point.z);
	vec3 normal_sized_third_point = vec3(100.0f * third_point.x, 100.0f * third_point.y, 100.0f * third_point.z);

	vec3 a = normal_sized_third_point - normal_sized_first_point;
	vec3 b = normal_sized_second_point - normal_sized_first_point;

	vec3 c = cross(a, b);

	return normalize(c);
}

void MeshModel::normalToFace()
{
	int i = 0;
	while(i < vertex_count) {
		vec3 curr_normal = calculateNormal(vertex_positions[i], vertex_positions[i + 1], vertex_positions[i + 2]);
		normals[i] = curr_normal;  // Store the normal vector
		normals[i+ 1] = (vertex_positions[i] + vertex_positions[i + 1] + vertex_positions[i + 2]) / 3.0f;
		i += 3;

	}
	//draw();
}

void MeshModel::calculateBoundingBox()
{
	
	GLfloat max_x = vertex_positions[0].x;
	GLfloat min_x = vertex_positions[0].x;

	GLfloat max_y = vertex_positions[0].y;
	GLfloat min_y = vertex_positions[0].y;

	GLfloat max_z = vertex_positions[0].z;
	GLfloat min_z = vertex_positions[0].z;

	int i = 0;
	while (i != vertex_count) {
		if (vertex_positions[i].x > max_x)
		{
			max_x = vertex_positions[i].x;
		} else if (vertex_positions[i].x < min_x)
		{
			min_x = vertex_positions[i].x;
		}
		if (vertex_positions[i].y > max_y)
		{
			max_y = vertex_positions[i].y;
		} else if (vertex_positions[i].y < min_y)
		{
			min_y = vertex_positions[i].y;
		}
		if (vertex_positions[i].z > max_z)
		{
			max_z = vertex_positions[i].z;
		}else if (vertex_positions[i].z < min_z)
		{
			min_z = vertex_positions[i].z;
		}
		i++;
	}
	bounding_box[0] = vec3(max_x, max_y, max_z);
	bounding_box[1] = vec3(max_x, max_y, min_z);
	bounding_box[2] = vec3(max_x, min_y, max_z);
	bounding_box[3] = vec3(max_x, min_y, min_z);
	bounding_box[4] = vec3(min_x, max_y, max_z);
	bounding_box[5] = vec3(min_x, max_y, min_z);
	bounding_box[6] = vec3(min_x, min_y, max_z);
	bounding_box[7] = vec3(min_x, min_y, min_z);
}