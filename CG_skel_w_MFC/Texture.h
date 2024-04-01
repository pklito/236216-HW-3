#pragma once

#include "Renderer.h"
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "stb_image.h"
#include "stdafx.h"

class Texture 
{
public:
	unsigned int m_RendererID;
	int m_width, m_height, m_BPP;

	Texture(const std::string& path);
	Texture() : m_RendererID(0), m_width(0), m_height(0), m_BPP(0) {};
	~Texture();

	void load(const std::string& path);
	void loadWoodTurbulenceTexture(int width, int height);
	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};