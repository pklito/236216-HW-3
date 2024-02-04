#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

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

/*
 This function gets two pixels on screen and draws the line between them (rasterization)
 vert1 + vert2 = ends of the edge
 normal = direction of normal.
*/
void Renderer::DrawLine(vec2 vert1, vec2 vert2){
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
			//inverted y and x (because we swapped them in the beginning)
			m_outBuffer[INDEX(m_width,y,x,0)]=1;	m_outBuffer[INDEX(m_width,y,x,1)]=1;	m_outBuffer[INDEX(m_width,y,x,2)]=1;
		}
		else{
			m_outBuffer[INDEX(m_width,x,y,0)]=1;	m_outBuffer[INDEX(m_width,x,y,1)]=1;	m_outBuffer[INDEX(m_width,x,y,2)]=1;
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
 * vertices: vector of the world space vertices
 * normals: directions of the respective world space normals.
 */
void Renderer::DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals)
{
	//if normals isn't supplied, give this iterator some garbage value (vertices->begin())
	vector<vec3>::const_iterator normal = normals != NULL ? normals->begin() : vertices->begin();
	for(auto it = vertices->begin(); it != vertices->end(); ++it, ++normal){
		/*
		TRANSFORMATIONS
		*/
		/*
		PROJECTIONS
		*/

		if(normals){
			//DrawLine(vert1, vert2, normal);
		}
		else{
			//DrawLine(vert1,vert2);
		}
	}
}

void Renderer::DrawPoint(const vec3 vertex)
{
    m_outBuffer[INDEX(m_width, toupper(100*vertex.x), toupper(100*vertex.y), 0)] = 0;
    m_outBuffer[INDEX(m_width, toupper(100*vertex.x), toupper(100*vertex.y), 1)] = 1;
    m_outBuffer[INDEX(m_width, toupper(100*vertex.x), toupper(100*vertex.y), 2)] = 0;
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