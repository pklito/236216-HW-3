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
	int vc[4]; // Color indices

	FaceIdcs()
	{
		for (int i = 0; i < 4; i++)
			v[i] = vn[i] = vt[i] = vc[i] = 0;
	}

	FaceIdcs(std::istream &aStream)
	{
		for (int i = 0; i < 4; i++)
			v[i] = vn[i] = vt[i] = vc[i] = 0;

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
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vc[i]; // Read color index
		}
	}
};

vec3 vec3fFromStream(std::istream &aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream &aStream)
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

	vector<vec3> vertex_colors;
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
		if (lineType == "v") { // BUG FIXED
			vertices.push_back(vec3fFromStream(issLine));
			vertex_colors.push_back(vec3(0.2, 0.3, 0.8));
		}
		else if (lineType == "f") // BUG FIXED
			faces.push_back(issLine);
		else if (lineType == "vn")
		{
			// normal to vertices
			vertex_normals.push_back(vec3fFromStream(issLine));
		}
		else if (lineType == "vt")
		{
			vertex_textures.push_back(vec3fFromStream(issLine));
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

	/*
	Convert the vectors read from the files to GLfloat arrays
	*/

	// array size is [3 * 3 * face_num], we have 3 * face_num vertices, and 3 coordinates for each.
	face_num = faces.size();
	GLfloat *vertices_array = new GLfloat[face_num * 3 * 3];
	GLfloat *vertex_normals_array = new GLfloat[face_num * 3 * 3];
	GLfloat *vertex_textures_array = new GLfloat[face_num * 3 * 3];

	GLfloat *vertex_colors_array = new GLfloat[face_num * 3 * 3];
	// convert
	int k = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int coord = 0; coord < 3; coord++)
			{ // x, y, z
				vertices_array[k + coord] = vertices[it->v[i] - 1][coord];
				vertex_colors_array[k + coord] = vertex_colors[it->v[i] - 1][coord];
				if (vertex_normals.size() > 0)
					vertex_normals_array[k + coord] = vertex_normals[it->vn[i] - 1][coord];
				if (vertex_textures.size() > 0)
					vertex_textures_array[k + coord] = vertex_textures[it->vt[i] - 1][coord]; // Take the face indexes from the vertix array BUG FIXED
			}
			// iterate to next face
			k += 3;
		}
	}
	// pass only the arrays that were filled.
	generateBuffers(vertices_array, vertex_colors_array, (vertex_normals.size() > 0) ? vertex_normals_array : nullptr, (vertex_textures.size() > 0) ? vertex_textures_array : nullptr, face_num);

	delete[] vertices_array;
	delete[] vertex_normals_array;
	delete[] vertex_textures_array;
	delete[] vertex_colors_array;
}

/// @brief This function handles taking arrays of vertices and sending them to the GPU.
///
/// If a parameter is not given (nullptr), an empty one will be generated/calculated
/// This populates: [vao, vert_vao, face_vao, box_vao], which handle [model draw, vertex normals draw, face normals draw, bounding box draw] respectively
/// @param vertices_array Existing GLfloat array of points. each 9 floats is a face. cannot be nullptr
/// @param vertex_normals_array Same order of vertices as {vertices_array}. if nullptr, face normals will be sent
/// @param vertex_textures_array Same order of vertices as {vertices_array}. texture coord for each vertex.
/// @param face_num Number of faces sent. all arrays should be [9 * facenum] length
void MeshModel::generateBuffers(const GLfloat *vertices_array, const GLfloat* vertex_colors_array, const GLfloat *vertex_normals_array, const GLfloat *vertex_textures_array, int face_num)
{
	
	// Generate a new array object for the main draw
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/*
	Handling nullptr inputs
	*/
	// Generate a new array instead, if nulls were inputted.
	GLfloat *vn_arr = vertex_normals_array != nullptr ? nullptr : new GLfloat[3 * 3 * face_num];
	GLfloat *vt_arr = vertex_textures_array != nullptr ? nullptr : new GLfloat[3 * 3 * face_num];

	// Generate vertex normals with the faces
	if (vertex_normals_array == nullptr)
	{
		// Caclulate normal per face
		for (int face = 0; face < face_num; face++)
		{
			vec3 p1 = vec3(vertices_array[9 * face + 0], vertices_array[9 * face + 1], vertices_array[9 * face + 2]);
			vec3 p2 = vec3(vertices_array[9 * face + 3], vertices_array[9 * face + 4], vertices_array[9 * face + 5]);
			vec3 p3 = vec3(vertices_array[9 * face + 6], vertices_array[9 * face + 7], vertices_array[9 * face + 8]);
			vec3 norm = calculateNormal(p1, p2, p3);

			for (int coord = 0; coord < 3; coord++)
			{
				vn_arr[9 * face + coord] = norm[coord];
				vn_arr[9 * face + 3 + coord] = norm[coord];
				vn_arr[9 * face + 6 + coord] = norm[coord];
			}
		}
	}
	if (vertex_textures_array == nullptr)
	{
		std::fill(vt_arr, vt_arr + (9 * face_num), 0);
		// TODO handle no textures
	}

	/*
	Generate the buffer objects to store these arrays
	*/
	GLuint vbos[4] = {0, 0, 0, 0};
	glGenBuffers(4, vbos);
	vbo_vertices = vbos[0];
	vbo_normals = vbos[1];
	vbo_textures = vbos[2];
	vbo_colors = vbos[3];

	// vertices, passed with location = 0
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 3,
				 vertices_array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// normals, passed with location = 1
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	if (vertex_normals_array)
		glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 3, vertex_normals_array, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 3, vn_arr, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)3);
	glEnableVertexAttribArray(1);

	// textures, passed with location = 2
	glBindBuffer(GL_ARRAY_BUFFER, vbo_textures);
	if (vertex_textures_array)
		glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 2, vertex_textures_array, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 2, vt_arr, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(2);

	// colors, passed with location = 3
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 3, vertex_colors_array, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(3);

	//
	// VERTEX NORMAL drawing vao
	//
	float NORMAL_LENGTH = 0.1;

	glGenVertexArrays(1, &vert_vao);
	glBindVertexArray(vert_vao);

	glGenBuffers(1, &vert_vbo);
	GLfloat *vert_points = new GLfloat[2 * 3 * 3 * face_num]; // Length is twice the amount of vertices, we store the end of their normal aswell
	for (int i = 0; i < 3 * face_num; i++)
	{
		for (int coord = 0; coord < 3; coord++)
		{
			// vertex point
			vert_points[3 * 2 * i + coord] = vertices_array[3 * i + coord];
			// normal end point
			if (vertex_normals_array)
				vert_points[3 * (2 * i + 1) + coord] = vertices_array[3 * i + coord] + NORMAL_LENGTH * vertex_normals_array[3 * i + coord];
			else
				vert_points[3 * (2 * i + 1) + coord] = vertices_array[3 * i + coord] + NORMAL_LENGTH * vn_arr[3 * i + coord];
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
	glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 3 * 2, vert_points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	delete[] vert_points;

	//
	// FACE NORMAL drawing vao
	//

	// Generate the vertex draw vao
	glGenVertexArrays(1, &face_vao);
	glBindVertexArray(face_vao);

	glGenBuffers(1, &face_vbo);
	GLfloat *norm_points = new GLfloat[2 * 3 * face_num];
	for (int face = 0; face < face_num; face++)
	{
		vec3 p1 = vec3(vertices_array[9 * face + 0], vertices_array[9 * face + 1], vertices_array[9 * face + 2]);
		vec3 p2 = vec3(vertices_array[9 * face + 3], vertices_array[9 * face + 4], vertices_array[9 * face + 5]);
		vec3 p3 = vec3(vertices_array[9 * face + 6], vertices_array[9 * face + 7], vertices_array[9 * face + 8]);
		vec3 norm = calculateNormal(p1, p2, p3);

		vec3 center = (p1 + p2 + p3) / 3;
		vec3 end = center + NORMAL_LENGTH * norm;

		for (int coord = 0; coord < 3; coord++)
		{
			norm_points[2 * 3 * face + coord] = center[coord];
			norm_points[2 * 3 * face + 3 + coord] = end[coord];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
	glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3 * 2,
				 norm_points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	delete[] norm_points;

	//
	// BOUNDING BOX drawing vao
	glGenVertexArrays(1, &box_vao);
	glBindVertexArray(box_vao);

	glGenBuffers(1, &box_vbo);

	float minX = vertices_array[0];
	float minY = vertices_array[1];
	float minZ = vertices_array[2];
	float maxX = vertices_array[0];
	float maxY = vertices_array[1];
	float maxZ = vertices_array[2];

	for (int i = 3; i < face_num * 3 * 3; i += 3) {
		minX = min(minX, vertices_array[i]);
		minY = min(minY, vertices_array[i + 1]);
		minZ = min(minZ, vertices_array[i + 2]);
		maxX = max(maxX, vertices_array[i]);
		maxY = max(maxY, vertices_array[i + 1]);
		maxZ = max(maxZ, vertices_array[i + 2]);
	}

	// Calculate bounding box vertices
	GLfloat bounding_box_vertices[] = {
		// Vertices of the bounding box
		minX, minY, minZ,  // Minimum corner
		maxX, minY, minZ,
		maxX, minY, minZ,
		maxX, minY, maxZ,
		maxX, minY, maxZ,
		minX, minY, maxZ,
		minX, minY, maxZ,
		minX, minY, minZ,
		// Repeat the same vertices for the top face
		minX, maxY, minZ,  // Maximum corner
		maxX, maxY, minZ,
		maxX, maxY, minZ,
		maxX, maxY, maxZ,
		maxX, maxY, maxZ,
		minX, maxY, maxZ,
		minX, maxY, maxZ,
		minX, maxY, minZ,
		// Connect the corners of the bounding box
		minX, minY, minZ,
		minX, maxY, minZ,
		maxX, minY, minZ,
		maxX, maxY, minZ,
		maxX, minY, maxZ,
		maxX, maxY, maxZ,
		minX, minY, maxZ,
		minX, maxY, maxZ
	};

	// Bind and fill the bounding box VBO with the vertices
	glBindBuffer(GL_ARRAY_BUFFER, box_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bounding_box_vertices), bounding_box_vertices, GL_STATIC_DRAW);

	// Specify the layout of the bounding box VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//
	/*
	glGenVertexArrays(1, &vert_vao);
	glBindVertexArray(vert_vao);

	glGenBuffers(1, &vert_vbo);
	GLfloat* vert_points = new GLfloat[2 * 3 * 3 * face_num];
	for(int i = 0; i < 3 * face_num; i ++){
		for(int coord = 0; coord < 3; coord ++){
			vert_points[3*2*i + coord] = vertices_array[3*i + coord];

		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo_textures);
	glBufferData(GL_ARRAY_BUFFER, face_num*sizeof(float)*3*3 * 2,
		vert_points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	delete[] vert_points;
	*/

	delete[] vt_arr;
	delete[] vn_arr;
	glBindVertexArray(0);
}

/// @brief
/// @param renderer
void MeshModel::draw(Renderer *renderer)
{
	mat4 full_trans = _world_transform * _model_transform;
	mat4 full_norm_trans = _world_normal_transform * _model_normal_transform;

	if (!draw_wireframe)
	{
		renderer->DrawMesh(vao, face_num, full_trans, full_norm_trans); // TODO: calculate this transform on change
	}
	else
	{
		renderer->DrawWireframe(vao, face_num, full_trans);
	}
	if (draw_vertex_normals)
	{
		renderer->DrawLines(vert_vao, 2 * 3 * face_num, full_trans, vec3(0.1, 0.1, 0.9));
	}
	if (draw_face_normals)
	{
		renderer->DrawLines(face_vao, 2 * face_num, full_trans, vec3(0, 1, 0.2));
	}
	// TODO: implement bounding box
	if (draw_bounding_box)
	{
		renderer->DrawLines(box_vao, 8*3, full_trans);
	}

}

mat4 MeshModel::getFullTransformation()
{
	return _world_transform * _model_transform;
}

void MeshModel::translate(GLfloat x_trans, GLfloat y_trans, GLfloat z_trans, bool isWorld)
{
	if (isWorld)
	{
		applyWorldTransformation(Translate(x_trans, y_trans, z_trans));
		applyWorldNormalTransformation(Translate(-x_trans, -y_trans, -z_trans)); // Normal Matrix = (M^-1)^T, we give it the inverse and the function will transform it
	}
	else
	{
		applyModelTransformation(Translate(x_trans, y_trans, z_trans));
		applyModelNormalTransformation(Translate(-x_trans, -y_trans, -z_trans));
	}
}

// TODO: Implement this function
void MeshModel::rotate(GLfloat theta_angle, int mode, bool isWorld)
{
	if (isWorld)
	{
		applyWorldTransformation(RotateAxis(theta_angle, mode));
		applyWorldNormalTransformation(RotateAxis(-theta_angle, mode));
	}
	else
	{
		applyModelTransformation(RotateAxis(theta_angle, mode));
		applyModelNormalTransformation(RotateAxis(-theta_angle, mode));
	}
}

// TODO: Implement this function
void MeshModel::scale(GLfloat x_scale, GLfloat y_scale, GLfloat z_scale, bool isWorld)
{
	if (isWorld)
	{
		applyWorldTransformation(Scale(x_scale, y_scale, z_scale));
		applyWorldNormalTransformation(Scale(1 / x_scale, 1 / y_scale, 1 / z_scale));
	}
	else
	{
		applyModelTransformation(Scale(x_scale, y_scale, z_scale));
		applyModelNormalTransformation(Scale(1 / x_scale, 1 / y_scale, 1 / z_scale));
	}
}

void MeshModel::applyWorldTransformation(const mat4 &transformation)
{
	_world_transform = transformation * _world_transform;
}

void MeshModel::applyModelTransformation(const mat4 &transformation)
{
	_model_transform = transformation * _model_transform;
}

void MeshModel::applyWorldNormalTransformation(const mat4 &transformation_inv)
{
	_world_normal_transform = transpose(transformation_inv) * _world_normal_transform;
}
void MeshModel::applyModelNormalTransformation(const mat4 &transformation_inv)
{
	_model_normal_transform = transpose(transformation_inv) * _model_normal_transform;
}

// TODO: Implement this function
void MeshModel::setShowNormals(bool change)
{
	draw_face_normals = change;
}

// TODO: Implement this function
void MeshModel::setShowNormalsToVertices(bool change)
{
	draw_vertex_normals = change;
}

// TODO: Implement this function
void MeshModel::setShowBox(bool change)
{
	draw_bounding_box = change;
}

// TODO: Implement this function
void MeshModel::setFillObj(bool fill)
{
	draw_wireframe = !fill;
}

// TODO: Implement this function
void MeshModel::changeColor()
{
	// Placeholder implementation
}

void MeshModel::setColor(float r, float g, float b)
{
	vector<vec3> vertex_colors;

	for (int i = 0; i < face_num * 3; ++i)
	{
		// Assuming each vertex has a color attribute index of 3 (modify accordingly)
		// Set RGB values for each vertex
		vertex_colors.push_back(vec3(r, g, b)); // Push RGB color to vertex_colors vector
	}

	GLfloat* vertex_colors_array = new GLfloat[face_num * 3 * 3];
	// Convert vertex_colors to vertex_colors_array
	int k = 0;
	for (const auto& color : vertex_colors)
	{
		for (int coord = 0; coord < 3; coord++)
		{ // x, y, z
			vertex_colors_array[k + coord] = color[coord];
		}
		// Iterate to next vertex
		k += 3;
	}
	
	// Update the vertex buffer object (VBO) with the new color data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors); // Assuming vbo_colors is the ID of the vertex color buffer object
	glBufferData(GL_ARRAY_BUFFER, face_num * sizeof(float) * 3, vertex_colors_array, GL_STATIC_DRAW);
	delete[] vertex_colors_array;
}

// TODO: Implement this function
void MeshModel::toggleSpecialMaterial()
{
	// Placeholder implementation
}

void MeshModel::resetToCenter()
{
	_model_transform = mat4();
	_world_transform = mat4();
}

//------------
// PRIM
//------------

void PrimMeshModel::Cube()
{
	const vec3 cube_points[] = {vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, -0.5f), vec3(-0.5f, 0.5f, -0.5f), vec3(-0.5f, -0.5f, 0.5f), vec3(0.5f, -0.5f, 0.5f), vec3(0.5f, 0.5f, 0.5f), vec3(-0.5f, 0.5f, 0.5f)};
	const int face_indices[] = {
		2, 1, 0,
		0, 3, 2,
		6, 5, 1,
		1, 2, 6,
		7, 4, 5,
		5, 6, 7,
		3, 0, 4,
		4, 7, 3,
		6, 2, 3,
		3, 7, 6,
		5, 4, 0,
		0, 1, 5};

	// Hardcoded cube vertices
	face_num = 12;

	static GLfloat vertices_array[12 * 3 * 3];
	static GLfloat vertex_colors_array[12 * 3 * 3];
	for (int i = 0; i < 3 * face_num; i++)
	{
		for (int coord = 0; coord < 3; coord++)
		{
			vertices_array[3 * i + coord] = cube_points[face_indices[i]][coord];
			// Set color to cyan for all vertices
			vertex_colors_array[3 * i + coord] = 0.0f; // Red
			vertex_colors_array[3 * i + coord + 1] = 1.0f; // Green
			vertex_colors_array[3 * i + coord + 2] = 1.0f; // Blue
		}
	}

	generateBuffers(vertices_array, vertex_colors_array, nullptr, nullptr, face_num);
	delete[] vertices_array;
	delete[] vertex_colors_array;
}

void PrimMeshModel::Tetrahedron()
{
#define base3 0, 0, -1
#define base2 -0.866, 0, 0.5
#define base1 0.866, 0, 0.5
#define top 0, 1, 0
	face_num = 4;
	static GLfloat vertices_array[3 * 4 * 3] = {base1, base2, base3, top, base1, base3, top, base2, base1, top, base3, base2};
	static GLfloat vertex_colors_array[3 * 4 * 3];

	// Fill vertex_colors_array with cyan color
	for (int i = 0; i < 3 * face_num * 3; i += 3)
	{
		vertex_colors_array[i] = 0.0f; // Red
		vertex_colors_array[i + 1] = 1.0f; // Green
		vertex_colors_array[i + 2] = 1.0f; // Blue
	}

	generateBuffers(vertices_array, vertex_colors_array, nullptr, nullptr, face_num);
}