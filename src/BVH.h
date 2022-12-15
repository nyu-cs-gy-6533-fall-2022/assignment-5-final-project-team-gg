#ifndef BVH
#define BVH

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

struct Triangle{
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
    int index;
};

class bvh{
public:
    glm::vec3 minv;
    glm::vec3 maxv;
    bool is_leaf;
    bvh* left;
    bvh* right;

};

#endif