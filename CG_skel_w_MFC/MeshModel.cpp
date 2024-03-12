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
	vertex_normals_exist = false;
	show_vertex_normals = false;
	show_face_normals = false;
	fill_obj = false;
	curr_color = 1;
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
	vector<vec3> normals_to_vert;
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
			normals_to_vert.push_back(vec3fFromStream(issLine));
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
	normals_to_vertices = new vec3[3 * faces.size()];
	
	std::cout << "[ ] Read model "<<vertices.size() << " VN:" <<  normals_to_vert.size() << std::endl;
	vertex_normals_exist = normals_to_vert.size() != 0;
	if(!vertex_normals_exist){
		vertex_normals_exist = true;
	}
	// iterate through all stored faces and create triangles
	int k = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			if(vertex_normals_exist){
				normals_to_vertices[k] = 0.1*normals_to_vert[it->vn[i] - 1] + vertices[it->v[i] - 1];
			}
			vertex_positions[k++] = vertices[it->v[i] - 1]; 	//Take the face indexes from the vertix array BUG FIXED
		}
	}

	calculateBoundingBox();
}

mat4 MeshModel::getWorldTransformation(){
	return _world_transform;
}

void MeshModel::draw(Renderer* renderer)
{
	std::vector<vec3> vec(vertex_positions, vertex_positions + (3 * face_count));

	if(vertex_normals_exist){
		std::vector<vec3> norm_to_vert(normals_to_vertices, normals_to_vertices + (3 * face_count));
		//material 
		vector<Material> matlist(1,material);	//one element
		if(special_color == 1){
			matlist.pop_back();
			for(auto& v : vec){
				matlist.push_back(Material(vec3(0.1,0.3,0.2),v,vec3(0.2,v.x/2,0.1),3));
			}
		}
		else if(special_color == 2){
			matlist.pop_back();
			for(auto& v : vec){
				matlist.push_back(Material(vec3(0.2,0.2,0.2),v,vec3(1,1,1)-v,5));
			}
		}
		renderer->DrawTriangles(&vec, _world_transform, &matlist, &norm_to_vert, show_face_normals, vec3(1,1,1), fill_obj);
	}
	else{
		vector<Material> matlist(1,material);
		renderer->DrawTriangles(&vec, _world_transform, &matlist, NULL, show_face_normals, vec3(1,1,1),fill_obj);
	}
	renderer->DrawBoundingBox(bounding_box, _world_transform, show_box);
}

void MeshModel::translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans)
{
	int i = 0;
	while (i < 3*face_count) {
		// Update the actual vertex positions in the array
		vertex_positions[i].x += x_trans;
		vertex_positions[i].y += y_trans;
		vertex_positions[i].z += z_trans;
		if(vertex_normals_exist){
			normals_to_vertices[i] =  normals_to_vertices[i] + vec3(x_trans,y_trans,z_trans);
		}
		i++;
	}
	calculateBoundingBox();
}

void MeshModel::scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale)
{
	if (x_scale == 0 || y_scale == 0 || z_scale == 0) {
		return;
	}

	int i = 0;
	while (i < 3*face_count) {

		vertex_positions[i].x *= x_scale;
		vertex_positions[i].y *= y_scale;
		vertex_positions[i].z *= z_scale;


		if(vertex_normals_exist){
		normals_to_vertices[i] =  normals_to_vertices[i] * vec3(x_scale,y_scale,z_scale);
		}
		i++;
	}
	calculateBoundingBox();
}

void MeshModel::rotate(GLfloat theta, int mode)
{
	if (theta == 0) {
		return;
	}
	//mode = 0 is around x, mode = 1 is around y, mode = 2 is around z
	int i = 0;
	mat4 rotation_matrix;
	if (mode == 0) {
		rotation_matrix = RotateX(theta);
	}
	else if (mode == 1) {
		rotation_matrix = RotateY(theta);
	}
	else if (mode == 2) {
		rotation_matrix = RotateZ(theta);
	}
	else {
		std::cout << "ERROR - ILLEGAL ROTATION PARAMETER:" << mode << std::endl;
		return;
	}
	while (i < 3*face_count) {
		vec3 current_vertex = vertex_positions[i];
		vec4 curr_rotated_point = vec4(rotation_matrix * vec4(current_vertex, 1.0f));
		
		vertex_positions[i] = vec3(curr_rotated_point.x, curr_rotated_point.y, curr_rotated_point.z);

		
		if(vertex_normals_exist){
			normals_to_vertices[i] = toVec3(rotation_matrix * normals_to_vertices[i]);
		}
		i++;
	}
	calculateBoundingBox();

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
	while(i < 3*face_count) {
		vec3 curr_normal = calculateNormal(vertex_positions[i], vertex_positions[i + 1], vertex_positions[i + 2]);
		normals[i] = curr_normal;  // Store the normal vector
		normals[i+ 1] = (vertex_positions[i] + vertex_positions[i + 1] + vertex_positions[i + 2]) / 3.0f;
		i += 3;

	}
	//draw();
}

void MeshModel::changeColor()
{
	curr_color = (curr_color + 1) % 7;
	updateMaterial();
}

void MeshModel::updateMaterial() {
	int num_of_color = curr_color % 7;
	switch (num_of_color) {
	case 0:
		material.color_diffuse = vec3(1, 1, 1);
		break;
	case 1:
		material.color_diffuse = vec3(0, 1, 0);
		break;
	case 2:
		material.color_diffuse = vec3(1, 0, 1);
		break;
	case 3:
		material.color_diffuse =  vec3(1, 1, 0);
		break;
	case 4:
		material.color_diffuse = vec3(0.4, 0, 0);
		break;
	case 5:
		material.color_diffuse = vec3(0, 0.5, 0);
		break;
	case 6:
		material.color_diffuse = vec3(0, 0, 0.5);
		break;
	}
}

void MeshModel::toggleSpecialMaterial(){
	special_color = (special_color + 1) %3;
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
	while (i != 3*face_count) {
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



void MeshModel::applyWorldTransformation(const mat4& transformation) {
	_world_transform = transformation * _world_transform;
}
void MeshModel::applyModelTransformation(const mat4& transformation) {
	//_model_transform = transformation * _model_transform;
}

void MeshModel::setShowNormals(bool change) 
{
	show_face_normals = change;
}

void MeshModel::setShowNormalsToVertices(bool change){
	show_vertex_normals = change;
}
void MeshModel::setShowBox(bool change)
{
	show_box = change;
}

vec3 MeshModel::calculateBoundingBoxCenter() 
{
	if (bounding_box == nullptr) {
		// Handle the case where the bounding box is not initialized
		return (0,0,0);
	}

	// Initialize min and max coordinates with the first point
	vec3 minCoord = bounding_box[0];
	vec3 maxCoord = bounding_box[0];

	// Find the minimum and maximum coordinates
	for (int i = 1; i < 8; ++i) {
		minCoord = min(minCoord, bounding_box[i]);
		maxCoord = max(maxCoord, bounding_box[i]);
	}

	// Calculate the center as the average of min and max coordinates
	return (minCoord + maxCoord) * 0.5f;
}

void MeshModel::setFillObj(bool fill)
{
	fill_obj = fill;
}

void MeshModel::resetToCenter() {
	// Set the model's position to the world center (0, 0, 0)
	vec3 bounding_box_center = calculateBoundingBoxCenter();
	translate(-bounding_box_center.x, -bounding_box_center.y, -bounding_box_center.z);
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
	face_count = 12;
	vertex_positions = new vec3[3*face_count];
	for(int i = 0; i < 3*face_count; i++){
		vertex_positions[i] = cube_points[face_indices[i]];
	}
}

void PrimMeshModel::Tetrahedron()
{
	face_count = 4;
	vertex_positions = new vec3[3*face_count];
	vec3 base1 = vec3( 0.866, 0, 0.5);
	vec3 base2 = vec3( -0.866, 0, 0.5);
	vec3 base3 = vec3( 0, 0, -1);
	vec3 top = vec3(0,1,0);

	int i = 0;
	vertex_positions[i++] = base3;
	vertex_positions[i++] = base2;
	vertex_positions[i++] = base1;

	vertex_positions[i++] = top;
	vertex_positions[i++] = base3;
	vertex_positions[i++] = base1;
	
	vertex_positions[i++] = top;
	vertex_positions[i++] = base1;
	vertex_positions[i++] = base2;
	
	vertex_positions[i++] = top;
	vertex_positions[i++] = base2;
	vertex_positions[i++] = base3;
	
}