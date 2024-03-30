#pragma once

#include "Renderer.h"
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "stb_image.h"
#include "stdafx.h"

class Texture 
{
	unsigned int m_RendererID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_width, m_height, m_BPP;
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};