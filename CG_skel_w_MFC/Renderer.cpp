#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
//#include "imgui.h"
#include "MeshModel.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define LINE_TOO_LARGE 30
Renderer::Renderer() :m_width(512), m_height(512)
{
	InitOpenGLRendering();
	CreateBuffers(512,512);
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
}



void Renderer::CreateBuffers(int width, int height)
{
	//ReleaseBuffers();
	m_width=width;
	m_height=height;	
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
}

void Renderer::ReleaseBuffers() {
	delete[] m_outBuffer;
	//delete[] m_zbuffer;

	m_outBuffer = nullptr;
	m_zbuffer = nullptr;
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,0)]=1;	m_outBuffer[INDEX(m_width,256,i,1)]=0;	m_outBuffer[INDEX(m_width,256,i,2)]=0;

	}
	//horizontal line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,0)]=1;	m_outBuffer[INDEX(m_width,i,256,1)]=0;	m_outBuffer[INDEX(m_width,i,256,2)]=1;

	}
}


void Renderer::ResizeBuffers(int new_width, int new_height) {
	if (new_width != m_width || new_height != m_height) {
		ReleaseBuffers();
		CreateBuffers(new_width, new_height);
	}
}

void Renderer::ClearBuffer(){
	std::fill(m_outBuffer,m_outBuffer+(m_width*m_height*3),0);
}

void Renderer::FillBuffer(float r, float g, float b)
{

	// Fill the buffer with the background color
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			DrawPixel(x,y,r,g,b);
		}
	}
}

void Renderer::FillEdges(float percent, float r, float g, float b){
	if(percent <= 0){
		return;
	}
	if(percent >= 0.5){
		percent = 0.5;
	}
	for(int i = 0; i <= (int)(m_height*percent); i++){
		for(int j = 0; j < m_width; j++){
			DrawPixel(j,i,r,g,b);
			DrawPixel(j,m_height - i- 1,r,g,b);
		}
	}
	for(int j = 0; j < (int)(m_width*percent); j++){
		for(int i = (int)(m_height*percent) - 1; i < m_height - (int)(m_height*percent) - 1; i++){
			DrawPixel(j,i,r,g,b);
			DrawPixel(m_width - j - 1,i,r,g,b);
		}
	}
}


/*
 This function gets two pixels on screen and draws the line between them (rasterization)
 vert1 + vert2 = ends of the edge
 normal = direction of normal.
*/
void Renderer::DrawLine(vec2 vert1, vec2 vert2, float r, float g, float b)
{
	//flip the axis so that slope is -1 <= m <= 1
	bool flipped = false;
	if (abs(vert1.y - vert2.y) > abs(vert1.x - vert2.x)) {
		auto temp = vert1.y;
		vert1.y = vert1.x;
		vert1.x = temp;

		temp = vert2.y;
		vert2.y = vert2.x;
		vert2.x = temp;
		flipped = true;
	}
	//swap the order so that vert1 is left of vert2
	if (vert1.x > vert2.x) {
		vec2 temp = vert1;
		vert1 = vert2;
		vert2 = temp;
	}

	//line drawing:
	int y = vert1.x <= vert2.x ? vert1.y : vert2.y;
	int dy = abs(vert2.y - vert1.y);
	int dx = vert2.x - vert1.x;
	int d = 2 * dy - dx;
	//increase or decrease y on move.
	int slope_direction = vert2.y >= vert1.y ? 1 : -1;

	for (int x = vert1.x; x <= vert2.x; x++)
	{
		if (d < 0) {
			d += 2 * dy;
		}
		else {
			y += slope_direction;
			d += 2 * dy - 2 * dx;
		}

		//light the pixel
		if(flipped){
			DrawPixelSafe(y,x, r, g, b);
		}
		else{
			DrawPixelSafe(x,y, r, g, b);
		}

	}
}

void Renderer::DrawPixel(int x, int y, float r, float g, float b){
	m_outBuffer[INDEX(m_width,x,y,0)]=r;	m_outBuffer[INDEX(m_width,x,y,1)]=g;	m_outBuffer[INDEX(m_width,x,y,2)]=b;
		
}

void Renderer::DrawPixelSafe(int x, int y, float r, float g, float b){
	if(x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;
	DrawPixel(x,y,r,g,b);
}

/**
 * This function takes a world space object, and draws it's triangles on screen.
 * 
 * This function
 * - converts object space to camera space (CameraTransform)
 * - converts camera space to screen space (3D to 2D)
 * - calls `DrawLine` to set the pixels on screen.
 * Parameters:
 * vertices: vector of the camera space vertices
 * normals: directions of the respective world space normals.
 */
void Renderer::DrawTriangles(const vector<vec3>* vertices, const mat4& world_transform, const vector<vec3>* edge_normals, bool draw_normals, float r, float g, float b)
{
	// Clear the buffer before drawing new content

	//if normals isn't supplied, give this iterator some garbage value (vertices->begin())
	vector<vec3>::const_iterator normal_it = edge_normals != NULL ? edge_normals->begin() : vertices->begin();
	for(auto it = vertices->begin(); it != vertices->end(); ++it, ++normal_it){
		//get the next face
		vec4 vert1 = vec4(*it);
		vec4 vert2 = vec4(*(it+1));
		vec4 vert3 = vec4(*(it+2));
		it = it + 2;

		vec4 normCoor1, normCoor2;

		vec4 vn1 = vec4(*normal_it);
		vec4 vn2 = vec4(*(normal_it+1));
		vec4 vn3 = vec4(*(normal_it+2));
		normal_it += 2;

		/*
		TRANSFORMATIONS + PROJECTION ( P * Tc-1 * v)
		*/
		vert1 = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vert1));
		vert2 = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vert2));
		vert3 = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vert3));

		vn1 = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vn1));
		vn2 = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vn2));
		vn3 = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vn3));

		if(vert1.z < -1 || vert1.z > 1 || vert2.z < -1 || vert2.z > 1 || vert3.z < -1 || vert3.z > 1){
			continue;
		}
		vec3 norm_dir = calculateNormal(toVec3(vert1),toVec3(vert2),toVec3(vert3))/5.f;
		normCoor1 = (vert1 + vert2 + vert3) / 3;
		normCoor2 = normCoor1 - norm_dir;


		//sometimes a point will get sent really far (matrix bs)
		//the DrawLine function wont draw out of bounds, but it will take
		//very long to go over the whole distance (~200,000 iterations).
		if (length(vert1) > LINE_TOO_LARGE || length(vert2) > LINE_TOO_LARGE || length(vert3) > LINE_TOO_LARGE)
		{
			continue;
		}

		float aspect_ratio = (float)(m_width)/(float)(m_height);
		/*
		Clipspace coordinates to screenspace coordinates
		*/
		vec2 p1 = vec2(RANGE(vert1.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(vert1.y,-1,1,0,m_height));
		vec2 p2 = vec2(RANGE(vert2.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(vert2.y,-1,1,0,m_height));
		vec2 p3 = vec2(RANGE(vert3.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(vert3.y,-1,1,0,m_height));

		vec2 n1 = vec2(RANGE(normCoor1.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(normCoor1.y, -1, 1, 0, m_height));
		vec2 n2 = vec2(RANGE(normCoor2.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(normCoor2.y, -1, 1, 0, m_height));


		DrawLine(p1, p2, r, g, b);
		DrawLine(p2, p3, r, g, b);
		DrawLine(p3, p1, r, g, b);

		if(edge_normals != NULL){
			vec2 a1 = vec2(RANGE(vn1.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(vn1.y,-1,1,0,m_height));
			vec2 a2 = vec2(RANGE(vn2.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(vn2.y,-1,1,0,m_height));
			vec2 a3 = vec2(RANGE(vn3.x,-aspect_ratio,aspect_ratio,0,m_width), RANGE(vn3.y,-1,1,0,m_height));
			DrawLine(p1, a1, 0, 0, b);
			DrawLine(p2, a2, 0, 0, b);
			DrawLine(p3, a3, 0, 0, b);
		}
		//Normal:
		if(draw_normals){
		  DrawLine(n1, n2, 1, 0, 1);
    	}
	}
}
vec2 normalizeVectorWithFixedPoint(const vec2& fixedPoint, const vec2& pointToNormalize)
{
	float directionX = pointToNormalize.x - fixedPoint.x;
	float directionY = pointToNormalize.y - fixedPoint.y;

	float length = std::sqrt(directionX * directionX + directionY * directionY);

	// Check if the length is not zero to avoid division by zero
	if (length > 0.0f) {
		// Normalize the direction vector and scale it to the original length
		return vec2(fixedPoint.x + directionX / length, fixedPoint.y + directionY / length);
	}

	// Return the original point if the length is zero
	return pointToNormalize;
}

void Renderer::DrawNormalsToVertices(const vector<vec3>* vertices, const vector<vec3>* vertex_normals, bool draw_normals)
{
	if (!vertex_normals) {
		return;
	}

	//if normals isn't supplied, give this iterator some garbage value (vertices->begin())
	vector<vec3>::const_iterator normal =  vertex_normals->begin();
	for (auto it = vertices->begin(); it != vertices->end(); it++, normal++) {
        // Get the next face
        vec4 vert1 = vec4(*it);
        vec4 normCoor = vec4(*normal);

        /*
        TRANSFORMATIONS + PROJECTION (P * Tc-1 * v)
        */

        vert1 = toEuclidian(mat_project * (mat_transform_inverse * vert1));
        normCoor = toEuclidian(mat_project * (mat_transform_inverse * (normCoor+vert1)));

        // Normalize the vector without applying the range
		//vec2 normalized_end_point = normalize(vec2(normCoor.x,normCoor.y)) + vec2(vert1.x,vert1.y);
		
		// Scale down the normalized vector (make the normals smaller)

		float aspect_ratio = (float)(m_width)/(float)(m_height);
		// Apply the range to the normalized point
		vec2 first_point = vec2(RANGE(normCoor.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(normCoor.y, -1, 1, 0, m_height));

		// Normal:
		if (draw_normals) {
			DrawLine(first_point, vec2(RANGE(vert1.x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(vert1.y, -1, 1, 0, m_height)), 0.2,0.5,1);
		}
    }
}

void Renderer::DrawBoundingBox(const vec3* bounding_box, const mat4& world_transform, bool draw_box) 
{
	if (!bounding_box || !draw_box) {
		return;
	}
	
	vec4 new_bounding_box[8];
	vec2 bounding_box_in_vectwo[8];
	for (int i = 0; i < 8; i++) {
		// Convert 3D point to homogeneous coordinates
		vec4 homogeneous_point = vec4(bounding_box[i], 1.0f);
		
		float aspect_ratio = (float)(m_width)/(float)(m_height);

		// Apply transformations
		new_bounding_box[i] = toEuclidian(mat_project * (mat_transform_inverse * world_transform * homogeneous_point));
		bounding_box_in_vectwo[i] = vec2(RANGE(new_bounding_box[i].x, -aspect_ratio, aspect_ratio, 0, m_width), RANGE(new_bounding_box[i].y, -1, 1, 0, m_height));
		//bounding_box_in_vectwo[i] = vec2(new_bounding_box[i].x, new_bounding_box[i].y);

	}
// Define the indices to connect vertices in a sequential manner
	const int indices[12][2] = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0},
		{4, 5}, {5, 7}, {7, 6}, {6, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	// Draw lines to connect the vertices of the bounding box using the defined indices
	for (int i = 0; i < 12; ++i) {
		if(new_bounding_box[indices[i][0]].z < -1 || new_bounding_box[indices[i][0]].z > 1 || 
		new_bounding_box[indices[i][1]].z < -1  || new_bounding_box[indices[i][0]].z > 1  ){
			continue;
		}
		//sometimes a point will get sent really far (matrix bs)
		//the DrawLine function wont draw out of bounds, but it will take
		//very long to go over the whole distance (~200,000 iterations).
		if (length(new_bounding_box[indices[i][0]]) > LINE_TOO_LARGE || length(new_bounding_box[indices[i][0]]) > LINE_TOO_LARGE)
		{
			continue;
		}
		DrawLine(bounding_box_in_vectwo[indices[i][0]], bounding_box_in_vectwo[indices[i][1]], 1, 1, 0);
	}
}

void Renderer::DrawSymbol(const vec3& vertex, const mat4& world_transform, SYMBOL_TYPE symbol, float scale,vec3 colors)
{
	scale *= 4;
	const std::vector<vec2> square_shape = {vec2(-1,-1),vec2(1,-1),	vec2(1,-1),vec2(1,1), vec2(1,1), vec2(-1,1), vec2(-1,1), vec2(-1,-1)};
	const std::vector<vec2> x_shape = {vec2(-1,-1),vec2(1,1),	vec2(1,-1),vec2(-1,1)};
	const std::vector<vec2> star_shape = {vec2(0,1),vec2(0,-1),	vec2(1,-1),vec2(-1,1), vec2(1,1), vec2(-1,-1), vec2(-1,0), vec2(1,0)};
	const std::vector<vec2> plus_shape = {vec2(0,1),vec2(0,-1),	vec2(-1,0),vec2(1,0)};
	
	const std::vector<vec2>* decided = &square_shape;

	vec4 screen_space = toEuclidian(mat_project * (mat_transform_inverse * world_transform * vertex));
	vec2 image_space = vec2(RANGE(screen_space.x, -1, 1, 0, m_width), RANGE(screen_space.y, -1, 1, 0, m_height));
	switch(symbol){
		case SYM_SQUARE:
			decided = &square_shape;
			break;
		case SYM_X:
			decided = &x_shape;
			break;
		case SYM_STAR:
			decided = &star_shape;
			break;
		case SYM_PLUS:
			decided = &plus_shape;
			break;
		default:
			decided = &x_shape;
		break;
	}


	auto a = decided->begin();
	while(a != decided->end()){
		DrawLine(scale*(*a) + image_space, scale*(*(a+1)) + image_space, colors.x, colors.y, colors.z);
		a+=2;
	}
}


void Renderer::SetCameraTransformInverse(const mat4& cTransform){
	mat_transform_inverse = cTransform;
}
void Renderer::SetProjection(const mat4& projection){
	mat_project = projection;
}

void Renderer::setCameraMatrixes(const mat4& cTransformInverse, const mat4& Projection){
	SetCameraTransformInverse(cTransformInverse);
	SetProjection(Projection);
}


void Renderer::Init(){
	CreateBuffers(m_width,m_height);

}


/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::SwapBuffers()
{
	UpdateBuffer();
	//clear the new buffer
	ClearBuffer();
}

//Doesn't clear the buffer afterwards!
void Renderer::UpdateBuffer(){

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();

}