#ifndef OBJECT_H_
#define OBJECT_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <iostream>

class Object{
public:
	Object(glm::vec3 coords = glm::vec3(0.0, 0.0, 0.0), glm::vec3 col = glm::vec3(1.0, 1.0, 0.0), unsigned int maxI = 0,
		bool reflecting = false, bool refracting = false,
		float ambientFactor = 0.2f, float specExponent = 50.0f) {
		this->coords = coords;
		color = col; 
		maxIndex = maxI;
		reflect = reflecting;
		refract = refracting;
		ambient = ambientFactor; 
		specularEx = specExponent; 
	};
	// intersection function: returns the closest intersection point with the given ray (or a negative value, if none exists)
	// output parameters: location of the intersection, object normal
	// PURE VIRTUAL FUNCTION: has to be implemented in all child classes.
	// virtual float intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& intersectPos, glm::vec3& normal) = 0;

	const glm::vec3& Color() { return color; };
	float AmbientFactor() { return ambient; };
	float SpecularExponent() { return specularEx; };
	bool Reflect() { return reflect; };

	void offset(glm::vec3 os) {
		coords += os;
		for (glm::vec3& i : this->vertices) {
			i += os;
	
			//if(i.y > -1)
			//	std::cout << i.x << " " << i.y << " " << i.z << std::endl;
		}
	}

	void adjustIndice(unsigned int os) {
		for (glm::ivec3& i : this->indices) {
			i += glm::ivec3(os, os, os);
		}
	}

	//// setter
	//void setVertices(std::vector<glm::vec3> V) { this->vertices = V; };
	//void setNormals(std::vector<glm::vec3> N) { this->normals = N; };
	//void setIndices(std::vector<glm::ivec3> T) { this->indices = T; };
	//void setTexCoords(std::vector<glm::vec2> TC) { this->texCoords = TC; };

	//// getter
	//glm::vec3 getCoords() { return this->coords; };
	//std::vector<glm::vec3> getVertices() { return this->vertices; }
	//std::vector<glm::vec3> getNormals() { return this->normals; }
	//std::vector<glm::ivec3> getIndices() { return this->indices; }
	//std::vector<glm::vec2> getTexCoords() { return this->texCoords; }
	glm::vec3 coords;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::ivec3> indices;
	std::vector<glm::vec2> texCoords;
	unsigned int maxIndex;

private:
	// object color
	glm::vec3 color;
	// basic material parameters
	float ambient;
	float specularEx;
	// is this object reflecting?
	bool reflect;
	bool refract;

	// buffer
};


class Torus : public Object {
private:
	float innerRadius;
	float outerRadius;
	unsigned int sector;
	unsigned int stack;
public:
	Torus(float iR, float oR, unsigned int sec, unsigned int sta) : Object() {
		innerRadius = iR;
		outerRadius = oR;
		sector = sec;
		stack = sta;
	}
};


class Sphere : public Object {
private:
	float radius;
	unsigned int sector;
	unsigned int stack;
public:
	Sphere(float r, unsigned int sec, unsigned int sta) : Object() {
		radius = r;
		sector = sec;
		stack = sta;
	}
};

class Cylinder : public Object {
private:
	float radius;
	float height;
	unsigned int sector;
};

class Capsule : public Object {
private:
	float radius;
	float height;
	unsigned int sector;
	unsigned int stack;
};

class Cone : public Object {
private:
	float radius;
	float height;
	unsigned int sector;
};

class TruncatedCone : public Object {
private:
	float topRadius;
	float baseRadius;
	float height;
	unsigned int sector;
};

#endif