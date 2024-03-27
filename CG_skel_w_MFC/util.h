#pragma once
#include "mat.h"

typedef enum {
	SYM_STAR,
	SYM_SQUARE,
	SYM_X,
	SYM_PLUS
} SYMBOL_TYPE;

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

	//Called with Material::weightedAverage(m1,m2,m3,w1,w2,w3);
	static Material weightedAverage(const Material& mat1, const Material& mat2, const Material& mat3, float w1, float w2, float w3){
	return Material(w1*mat1.color_ambient + w2*mat2.color_ambient + w3*mat3.color_ambient, 
					w1*mat1.color_diffuse + w2*mat2.color_diffuse + w3*mat3.color_diffuse,
					w1*mat1.color_specular + w2*mat2.color_specular + w3*mat3.color_specular,
					int(w1*mat1.k_shiny + w2*mat2.k_shiny + w3*mat3.k_shiny) );
	}
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

	virtual void translate(float x, float y, float z) {}
	virtual void rotate(float angle, int axis) {}
	virtual void scale(float x, float y, float z) {}
	virtual CString getName(){ return _T("Generic Light");}

};

class PointLight : public Light {
protected:
	vec3 position;

public:
	//PointLight() : Light(0, vec3(0,0,0)), position(0,0,0) {}
	PointLight(float intensity, const vec3& color,vec3 position) : Light(intensity, color), position(position) {}

	void setPosition(const vec3& postion) {this->position = position;}
	vec3 getPosition() {return position;}

	virtual void translate(float x, float y, float z) override {position += (vec3(x,y,z));}
	virtual void rotate(float angle, int axis) override {}
	virtual void scale(float x, float y, float z) override {intensity *= x;}
	virtual CString getName() override { return _T("Point Light");}
};

class DirectionalLight : public Light {
protected:
	vec3 direction;

public:
	//DirectionalLight() : Light(0,vec3(0,0,0)), direction(vec3(0,0,0)) {}
	DirectionalLight(float intensity, const vec3& color,vec3 direction) : Light(intensity, color), direction(direction) {}

	void setDirection(const vec3& direction) {this->direction = normalize(direction);}
	vec3 getDirection() {return direction;}

	virtual void translate(float x, float y, float z) override {}
	virtual void rotate(float angle, int axis) override {setDirection(toVec3(RotateAxis(angle,2-axis)*vec4(direction)));}
	virtual void scale(float x, float y, float z) override {intensity *= x;}

	virtual CString getName() override { return _T("Directional Light");}
};

class AmbientLight : public Light {
public:
	AmbientLight() : Light(0,vec3(0,0,0)) {}
	AmbientLight(float intensity, const vec3& color) : Light(intensity, color) {}
	virtual CString getName() override { return _T("Ambient Light");}
};

////////////// TRIPLE

template<typename TYPE>
class Triple {
public:
    // Constructors
    Triple() : first(), second(), third() {}
    Triple(const TYPE& f, const TYPE& s, const TYPE& t) : first(f), second(s), third(t) {}

	/**
    // Accessors
    const TYPE& First() const { return first; }
    TYPE& First() { return first; }
    const TYPE& Second() const { return second; }
    TYPE& Second() { return second; }
    const T1& Third() const { return third; }
    TYPE& Third() { return third; }

	void setFirst()
	*/
public:
    TYPE first;
    TYPE second;
	TYPE third;
};