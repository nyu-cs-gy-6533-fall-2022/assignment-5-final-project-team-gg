#ifndef OBJECT_H_
#define OBJECT_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

class Object{
public:
	Object(glm::vec3 col = glm::vec3(1.0, 1.0, 0.0), bool reflecting = false, bool refracting = false, float ambientFactor = 0.2f, float specExponent = 50.0f) {
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
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::ivec3> indices;
	std::vector<glm::vec2> texCoords;
};


#endif