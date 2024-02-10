#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
//#include "imgui.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c

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
	m_width=width;
	m_height=height;	
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
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

void Renderer::ClearBuffer()
{
	// Assuming you have a background color (adjust the values accordingly)
	vec3 background_color = vec3(0.0, 0.0, 0.0);

	// Fill the buffer with the background color
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			m_outBuffer[INDEX(m_width, x, y, 0)] = background_color.x;
			m_outBuffer[INDEX(m_width, x, y, 1)] = background_color.y;
			m_outBuffer[INDEX(m_width, x, y, 2)] = background_color.z;
		}
	}
}

/*
 This function gets two pixels on screen and draws the line between them (rasterization)
 vert1 + vert2 = ends of the edge
 normal = direction of normal.
*/
void Renderer::DrawLine(vec2 vert1, vec2 vert2, int specialColor, bool clear){
	//Temp solution. drawing a line out of bounds crashes the code!
	if(vert1.x < 1 || vert2.x < 1 || vert1.x >= m_width-1 || vert2.x >= m_width-1 ||
	   vert1.y < 1 || vert2.y < 1 || vert2.y >= m_height-1 || vert1.y >= m_height-1){
		return;
	   }
	//flip the axis so that slope is -1 <= m <= 1
	bool flipped = false;
	if(abs(vert1.y-vert2.y) > abs(vert1.x - vert2.x)){
		auto temp = vert1.y;
		vert1.y = vert1.x;
		vert1.x = temp;

		temp = vert2.y;
		vert2.y = vert2.x;
		vert2.x = temp;
		flipped = true;
	}
	//swap the order so that vert1 is left of vert2
	if(vert1.x > vert2.x){
		vec2 temp = vert1;
		vert1 = vert2;
		vert2 = temp;
	}

	//line drawing:
	int y = vert1.x <= vert2.x ? vert1.y : vert2.y;
	int dy = abs(vert2.y - vert1.y);
	int dx = vert2.x - vert1.x;
	int d = 2*dy - dx;
	//increase or decrease y on move.
	int slope_direction = vert2.y >= vert1.y ? 1 : -1;

	for(int x = vert1.x; x <= vert2.x; x++){
		if(d < 0){
			d+=2*dy;
		}
		else{
			y += slope_direction;
			d+=2*dy - 2*dx;
		}

		//light the pixel
		if(flipped){
			if (specialColor == 1) {
				m_outBuffer[INDEX(m_width, y, x, 0)] = 0;	m_outBuffer[INDEX(m_width, y, x, 1)] = 1;	m_outBuffer[INDEX(m_width, y, x, 2)] = 1;
			}else if (specialColor == 2) {
				m_outBuffer[INDEX(m_width, y, x, 0)] = 1;	m_outBuffer[INDEX(m_width, y, x, 1)] = 0;	m_outBuffer[INDEX(m_width, y, x, 2)] = 0;
			} else {
				m_outBuffer[INDEX(m_width, y, x, 0)] = 1;	m_outBuffer[INDEX(m_width, y, x, 1)] = 1;	m_outBuffer[INDEX(m_width, y, x, 2)] = 1;
			}

			// clear the pixel if specified
			if (clear)
			{
				m_outBuffer[INDEX(m_width, y, x, 0)] = 0; // set to background color
				m_outBuffer[INDEX(m_width, y, x, 1)] = 0;
				m_outBuffer[INDEX(m_width, y, x, 2)] = 0;
			}
			//inverted y and x (because we swapped them in the beginning)
		}
		else{
			if (specialColor == 1) {
				m_outBuffer[INDEX(m_width, x, y, 0)] = 0;	m_outBuffer[INDEX(m_width, x, y, 1)] = 1;	m_outBuffer[INDEX(m_width, x, y, 2)] = 1;
			} else if (specialColor == 2) {
				m_outBuffer[INDEX(m_width, x, y, 0)] = 1;	m_outBuffer[INDEX(m_width, x, y, 1)] = 0;	m_outBuffer[INDEX(m_width, x, y, 2)] = 0;
			} else {
				m_outBuffer[INDEX(m_width, x, y, 0)] = 1;	m_outBuffer[INDEX(m_width, x, y, 1)] = 1;	m_outBuffer[INDEX(m_width, x, y, 2)] = 1;
			}
			// clear the pixel if specified
			if (clear)
			{
				m_outBuffer[INDEX(m_width, x, y, 0)] = 0; // set to background color
				m_outBuffer[INDEX(m_width, x, y, 1)] = 0;
				m_outBuffer[INDEX(m_width, x, y, 2)] = 0;
			}
		}
			
	}
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
void Renderer::DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals, bool draw_normals)
{
	// Clear the buffer before drawing new content
	ClearBuffer();
	//if normals isn't supplied, give this iterator some garbage value (vertices->begin())
	vector<vec3>::const_iterator normal = normals != NULL ? normals->begin() : vertices->begin();
	for(auto it = vertices->begin(); it != vertices->end(); ++it, ++normal){
		//get the next face
		vec4 vert1 = vec4(*it);
		vec4 vert2 = vec4(*(it+1));
		vec4 vert3 = vec4(*(it+2));
		it = it + 2;

		if (normals) {
			std::cout << "NORMALS IS: " << *normal << std::endl;
		}
		vec4 normCoor1 = vec4(*normal);
		vec4 normCoor2 = normCoor1 + vec4(*(normal + 1));
		
		normal = normal + 2;
		
		/*
		TRANSFORMATIONS + PROJECTION ( P * Tc-1 * v)
		*/
		vert1 = toEuclidian(mat_project * (mat_transform_inverse * vert1));
		vert2 = toEuclidian(mat_project * (mat_transform_inverse * vert2));
		vert3 = toEuclidian(mat_project * (mat_transform_inverse * vert3));

		normCoor1 = toEuclidian(mat_project * (mat_transform_inverse * normCoor1));
		normCoor2 = toEuclidian(mat_project * (mat_transform_inverse * normCoor2));
		/*
		Clipspace coordinates to screenspace coordinates
		*/
		vec2 p1 = vec2(RANGE(vert1.x,-1,1,0,m_width), RANGE(vert1.y,-1,1,0,m_height));
		vec2 p2 = vec2(RANGE(vert2.x,-1,1,0,m_width), RANGE(vert2.y,-1,1,0,m_height));
		vec2 p3 = vec2(RANGE(vert3.x,-1,1,0,m_width), RANGE(vert3.y,-1,1,0,m_height));
	
		vec2 n1 = vec2(RANGE(normCoor1.x, -1, 1, 0, m_width), RANGE(normCoor1.y, -1, 1, 0, m_height));
		vec2 n2 = vec2(RANGE(normCoor2.x, -1, 1, 0, m_width), RANGE(normCoor2.y, -1, 1, 0, m_height));

		std::cout << "not here" << std::endl;
		if(normals){
			if (draw_normals) {
				DrawLine(n1, n2, 1, false);
			} else {
				DrawLine(n1, n2, 1, true);
			}
			std::cout << p1 << " " << p2 << " " << p3 << std::endl;
			DrawLine(p1, p2, 0, false);

			std::cout << "or here" << std::endl;
			DrawLine(p2, p3, 0, false);
			DrawLine(p3, p1, 0, false);
			//DrawLine(vert1,vert2);
			//DrawLine(vert1, vert2, normal);
		}
		else{
			std::cout << p1 << " " << p2 << " " << p3 << std::endl;
			DrawLine(p1, p2, 0, false);
			
		std::cout << "or here" << std::endl;
			DrawLine(p2, p3, 0, false);
			DrawLine(p3, p1, 0, false);
			//DrawLine(vert1,vert2);
		}
	}

	std::cout << "DONE!" << std::endl;
}

void Renderer::DrawBoundingBox(const vec3* bounding_box, bool draw_box) 
{
	if (!bounding_box || draw_box == false) {
		return;
	}
	
	for (int i = 0; i < 8; i++) {
		std::cout << "BEFORE TRANSFORMATION " << i << ": " << bounding_box[i] << std::endl;
	}
	vec4 new_bounding_box[8];
	vec2 bounding_box_in_vectwo[8];
	for (int i = 0; i < 8; i++) {
		// Convert 3D point to homogeneous coordinates
		vec4 homogeneous_point = vec4(bounding_box[i], 1.0f);

		// Apply transformations
		new_bounding_box[i] = toEuclidian(mat_project * (mat_transform_inverse * homogeneous_point));
		bounding_box_in_vectwo[i] = vec2(RANGE(new_bounding_box[i].x, -1, 1, 0, m_width), RANGE(new_bounding_box[i].y, -1, 1, 0, m_height));
		//bounding_box_in_vectwo[i] = vec2(new_bounding_box[i].x, new_bounding_box[i].y);
		std::cout << "TRYING TO DRAW THE STUPID BOX! 1 bounding box in vectwo i:" << i << " --- " << bounding_box_in_vectwo[i] << std::endl;
	}
	for (int i = 0; i < 8; i++) {
		std::cout << "Transformed Point " << i << ": " << new_bounding_box[i] << std::endl;
	}
	std::cout << "TRYING TO DRAW THE STUPID BOX! 2" << std::endl;
	std::cout << "bounding_box at 1 is: " << bounding_box_in_vectwo[1] << "bounding_box at 5 is: " << bounding_box_in_vectwo[5] << std::endl;
	DrawLine(bounding_box_in_vectwo[1], bounding_box_in_vectwo[5], 2);
	DrawLine(bounding_box_in_vectwo[1], bounding_box_in_vectwo[0], 2);
	DrawLine(bounding_box_in_vectwo[1], bounding_box_in_vectwo[3], 2);
	DrawLine(bounding_box_in_vectwo[2], bounding_box_in_vectwo[0], 2);
	DrawLine(bounding_box_in_vectwo[2], bounding_box_in_vectwo[6], 2);
	DrawLine(bounding_box_in_vectwo[2], bounding_box_in_vectwo[3], 2);
	DrawLine(bounding_box_in_vectwo[3], bounding_box_in_vectwo[7], 2);
	DrawLine(bounding_box_in_vectwo[4], bounding_box_in_vectwo[5], 2);
	DrawLine(bounding_box_in_vectwo[4], bounding_box_in_vectwo[6], 2);
	DrawLine(bounding_box_in_vectwo[4], bounding_box_in_vectwo[0], 2);
	DrawLine(bounding_box_in_vectwo[5], bounding_box_in_vectwo[7], 2);
	DrawLine(bounding_box_in_vectwo[6], bounding_box_in_vectwo[7], 2);
}

void Renderer::DrawPoint(const vec3& vertex)
{
	std::cout << vertex << std::endl;
   m_outBuffer[INDEX(m_width, CLAMP(toupper(100*vertex.x),0,m_width), CLAMP(toupper(100*vertex.y),0,m_height),0)] = 0;
   m_outBuffer[INDEX(m_width, CLAMP(toupper(100*vertex.x),0,m_width), CLAMP(toupper(100*vertex.y),0,m_height),1)] = 1;
   //m_outBuffer[INDEX(m_width, toupper(100*vertex.x), toupper(100*vertex.y), 2)] = 0;
}


void Renderer::SetCameraTransformInverse(const mat4& cTransform){
	mat_transform_inverse = cTransform;
}
void Renderer::SetProjection(const mat4& projection){
	mat_project = projection;
}

void Renderer::Init(){
	CreateBuffers(m_width,m_height);

	/*// Initialize ImGui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	// Setup ImGui style
	//ImGui::StyleColorsDark();*/
}

/*void Renderer::Render() {
	// Your rendering code goes here

	// Render ImGui
	ImGui::Render();
}

void Renderer::HandleInput() {
	// Your input handling code goes here

	// Example: Check if the left arrow button is pressed
	if (ImGui::ArrowButton("RotateLeft", ImGuiDir_Left)) {
		// Code to rotate the object left goes here
	}
}*/




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