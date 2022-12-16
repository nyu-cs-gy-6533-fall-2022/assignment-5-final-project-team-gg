#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <iostream>

#include "Helpers.h"

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
    float lifetime;

    Particle() : position(0.0f), velocity(0.0f), color(1.0f), lifetime(0.0f) {}
};

class ParticleGenerator {
public:
    ParticleGenerator(Program program, unsigned int numParticles)
        : program(program), numParticles(numParticles) {

        this->init();
    }

    void Update(float dt, unsigned int newParticles, 
        glm::vec2 offset = glm::vec2(0.0f)) {
        
        for (unsigned int i=0; i<newParticles; i++) {
            int unusedParticle = this->firstUnusedParticle();
            this->respawnParticle(this->particles[unusedParticle], offset);
        }

        // UPDATE Particles
        for (unsigned int i=0; i<this->numParticles; i++) {
            Particle& p = this->particles[i];
            p.lifetime -= dt;
            if (p.lifetime > 0.0f) {
                p.position += p.velocity*dt;
                p.color.a -= dt * 2.5f; // DECAY RATE
            }
        }
    }

    void Draw() {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        this->program.bind();
        for (Particle p : this->particles) {
            if (p.lifetime > 0.0f) {
                glUniform2f(program.uniform("offset"), p.position.x, p.position.y);
                glUniform4f(program.uniform("color"), p.color.x, p.color.y, p.color.z, p.color.w);
                this->VAO.bind();
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            }
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

private:
    glm::vec2 position;
    std::vector<Particle> particles;
    unsigned int numParticles;
    unsigned int lastUsedParticle = 0;
    Program program;
    VertexArrayObject VAO;

    void init() {
        BufferObject VBO;
        
        float pQuad[] = {
            0.0f, 1.0f,   0.0f, 1.0f,
            1.0f, 0.0f,   1.0f, 0.0f,
            0.0f, 0.0f,   0.0f, 0.0f, 

            0.0f, 1.0f,   0.0f, 1.0f, 
            1.0f, 1.0f,   1.0f, 1.0f,
            1.0f, 0.0f,   1.0f, 0.0f
        };

        this->VAO.init();
        VBO.init();
        this->VAO.bind();
        VBO.bind();
        glBufferData(GL_ARRAY_BUFFER, sizeof(pQuad), pQuad, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glBindVertexArray(0);

        for (unsigned int i=0; i<this->numParticles; i++) {
            this->particles.push_back(Particle());
        }
    }

    unsigned int firstUnusedParticle() {
        for (unsigned int i=this->lastUsedParticle; i<this->numParticles; i++) {
            if (this->particles[i].lifetime <= 0.0f) {
                this->lastUsedParticle = i;
                return i;
            }
        }
        for (unsigned int i=0; i<this->lastUsedParticle; i++) {
            if (this->particles[i].lifetime <= 0.0f) {
                lastUsedParticle = i;
                return i;
            }
        }
        this->lastUsedParticle = 0;
        return 0;
    }

    void respawnParticle(Particle& p, glm::vec2 offset = glm::vec2(0.0f)) {
        float r = ((rand() % 100) - 50) / 10.0f;
        float rColor = 0.5f + ((rand() % 100) / 100.0f);
        p.position = this->position + offset + r;
        p.color = glm::vec4(rColor, rColor, rColor, 1.0f);
        p.lifetime = 1.0f;
        p.velocity = glm::vec2(1.0f);
    }
};
#endif