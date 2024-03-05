#pragma once

class Material {
public:
	const float k_ambient;
	const float k_diffuse;
	const float k_specular;
	const int k_shiny;
	Material() : k_ambient(0.2), k_diffuse(0.8), k_specular(0.5), k_shiny(1) {};
	Material(float ambient, float diffuse, float specular, int shiny) : k_ambient(ambient), k_diffuse(diffuse), k_specular(specular), k_shiny(shiny) {};
	void changeAmbient(float new_ambient);
	void changeDiffuse(float diffuse);
	void changeSpecular(float new_specular);
	void changeShinyness(int new_shiny);
};

class Light {
public:
	vec3 position;
	vec3 direction;
	vec3 color;
	float intensity;

	void setDir(vec3 dir) { direction = dir; }
	void setPos(vec3 pos) { position = pos; }
	void setColor(vec3 new_color) { color = new_color; }
	void setIntensity(float new_intensity) { intensity = new_intensity; }

	vec3 getLightPosition() { return position; }
};
