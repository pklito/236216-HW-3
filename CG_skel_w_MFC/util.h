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
protected:
	vec3 color;
	float intensity;
public:
	Light(float intensity, const vec3& color): color(color), intensity(intensity){}
	Light() : color(vec3(1,1,1)), intensity(1){}
	void setColor(const vec3& color) {this->color = color;}
	vec3 getColor() {return color;}
	void setIntensity(float intensity) {this->intensity = intensity;}
	float getIntensity() {return intensity;}


};

class PointLight : public Light {
protected:
	vec3 position;

public:
	PointLight(float intensity, const vec3& color,vec3 position) : Light(intensity, color), position(position) {}

	void setPosition(const vec3& postion) {this->position = position;}
	vec3 getPosition() {return position;}
};

class DirectionalLight : public Light {
protected:
	vec3 direction;

public:
	DirectionalLight(float intensity, const vec3& color,vec3 direction) : Light(intensity, color), direction(direction) {}

	void setDirection(const vec3& postion) {this->direction = direction;}
	vec3 getDirection() {return direction;}
};

class AmbientLight : public Light {
public:
	AmbientLight(float intensity, const vec3& color) : Light(intensity, color) {}
}