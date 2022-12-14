#ifndef OBJECT_H_
#define OBJECT_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <iostream>

class Light {
public:
    Light(int identifier = 0, float Ia = 0.2f, float Ii = 1.0f) {
        this->identifier = identifier;
        I_a = Ia;
        I_i = Ii;
    }
    // 0: direction, 1: Point, 2: Spot, 3: area
    int identifier;

    float I_a;
    float I_i;

    std::vector<glm::vec3> vertice;
    glm::vec3 dir;
};

// 1: identifier 4: vertex 1: direction

class DirectionalLight : public Light {
public:
    glm::vec3 direction;
    DirectionalLight(glm::vec3 d) : Light() {
        identifier = 0;
        for (int i = 0; i < 4; i++) vertice.push_back(glm::vec3(0, 0, 0));
        direction = d;
    }
};


class PointLight : public Light {
public:
    glm::vec3 position;
    PointLight(glm::vec3 p) : Light() {
        identifier = 1;

        position = p;
        vertice.push_back(position);
        for (int i = 0; i < 3; i++) vertice.push_back(glm::vec3(0, 0, 0));


    }
};


class SpotLight : public Light {
public:
    glm::vec3 postion;
    glm::vec3 direction;
    float angle;
    SpotLight(glm::vec3 p, glm::vec3 d, float a) : Light() {
        postion = p;
        direction = d;
        angle = a;
    }
};


class Arealight : public Light {
public:
    std::vector<glm::vec3> vertex;

};


#endif