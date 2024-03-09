#pragma once
class Material {
public:
	vec3 color_ambient;
	vec3 color_diffuse;
	vec3 color_specular;
	int k_shiny;
	Material() : color_ambient(vec3(0.2)), color_diffuse(vec3(0.8)), color_specular(vec3(0.5)), k_shiny(5) {};
	Material(const vec3& ambient, const vec3& diffuse, const vec3& specular, int shiny) : color_ambient(ambient), color_diffuse(diffuse), color_specular(specular), k_shiny(shiny) {};

	Material(const Material& mat)  : color_ambient(mat.color_ambient), color_diffuse(mat.color_diffuse), color_specular(mat.color_specular), k_shiny(mat.k_shiny) {};
	void operator=(const Material& mat) {color_ambient = mat.color_ambient; color_diffuse = mat.color_diffuse; color_specular = mat.color_specular; k_shiny = mat.k_shiny;};
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
	virtual float getIntensity() {return intensity;}


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

	void setDirection(const vec3& direction) {this->direction = direction;}
	vec3 getDirection() {return direction;}
};

class AmbientLight : public Light {
public:
	AmbientLight(float intensity, const vec3& color) : Light(intensity, color) {}
};