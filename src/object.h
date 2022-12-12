#ifndef OBJECT_H_
#define OBJECT_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

class Object{
public:
	Object(float x = 0.0f, float y = 0.0f, float z = 0.0f, glm::vec3 col = glm::vec3(1.0, 1.0, 0.0), bool reflecting = false, bool refracting = false,
		float ambientFactor = 0.2f, float specExponent = 50.0f) {
		this->x = x;
		this->y = y;
		this->z = z;
		color = col; 
		reflect = reflecting;
		refract = refracting;
		ambient = ambientFactor; 
		specularEx = specExponent; 
	};
	// intersection function: returns the closest intersection point with the given ray (or a negative value, if none exists)
	// output parameters: location of the intersection, object normal
	// PURE VIRTUAL FUNCTION: has to be implemented in all child classes.
	virtual float intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& intersectPos, glm::vec3& normal) = 0;

	const glm::vec3& Color() { return color; };
	float AmbientFactor() { return ambient; };
	float SpecularExponent() { return specularEx; };
	bool Reflect() { return reflect; };

	void setVertices(std::vector<glm::vec3> V) { this->vertices = V; };
	void setNormals(std::vector<glm::vec3> N) { this->normals = N; };
	void setIndices(std::vector<glm::ivec3> T) { this->indices = T; };
	void setTexCoords(std::vector<glm::vec2> TC) { this->texCoords = TC; };

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
	glm::vec3 coords;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::ivec3> indices;
	std::vector<glm::vec2> texCoords;
};


class Torus : public Object {
private:
	float innerRadius;
	float outerRadius;
	unsigned int sector;
	unsigned int stack;
};


class Sphere : public Object {
private:
	float radius;
	unsigned int sector;
	unsigned int stack;
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