#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Boid.h"
#include "Shape.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Bounds.h"
#include "Obstacle.h"

using namespace std;
using namespace glm;

int Boid::ID_COUNT = 0; // declared in header, but needs to be defined (here)

Boid::Boid(vec3 _pos) :
    id(ID_COUNT),
    pos(_pos),
    velocity(0.0f, 0.0f, 0.0f),
    influenceDist(0.66f),
    r(0.03f) {
    ID_COUNT++;
}

void Boid::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> prog) const {
    if (shape) {
        if (targetEnabled)
            glUniform3fv(prog->getUniform("kdFront"), 1, glm::value_ptr(vec3(1.0, 0.0, 0.0)));
        else
            glUniform3fv(prog->getUniform("kdFront"), 1, glm::value_ptr(vec3(0.0, 0.0, 1.0)));
        MV->pushMatrix();
        MV->translate(pos);
        MV->scale(r);
        glm::mat4 g = glm::transpose(glm::lookAt(vec3(0.0f), velocity, vec3(0.0f, 1.0f, 0.0f)));
        glm::mat4 E = glm::mat4_cast(glm::normalize(glm::toQuat(g)));
        MV->multMatrix(E);
        MV->rotate(radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
        //MV->multMatrix(glm::lookAt(vec3(0.0f), velocity, vec3(0.0f, 1.0f, 0.0f)));
        //MV->rotate(radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        shape->draw(prog);
        MV->popMatrix();
    }
}

void Boid::update(float timestep) {
    float maxSpeed = 5.0f;
    float minSpeed = 0.75f;
    vec3 accel(0.0f);
    accel += rule1();
    accel += rule2();
    accel += rule3();
    accel += moveTarget2();
    accel += stayBounds();
    mtxV.lock();
    velocity += accel * timestep / (float)(5e-3); // accel values balanced around 5e-3 timestep
    if (length(velocity) > maxSpeed) velocity = maxSpeed * normalize(velocity);
    else if (length(velocity) < minSpeed) velocity = minSpeed * normalize(velocity);
    mtxV.unlock();
    mtxP.lock();
    pos += timestep * velocity;
    mtxP.unlock();
}

vec3 Boid::getVelocity() {
    mtxV.lock();
    vec3 vel = velocity;
    mtxV.unlock();
    return vel;
}

vec3 Boid::getPos() {
    mtxP.lock();
    vec3 p = pos;
    mtxP.unlock();
    return p;
}

vec3 Boid::rule1() {
    // rule 1: separation
    // if within a certain distance of another boid, move away
    float r1Dist = 2.0f * r; // default 2 * r
    vec3 repellant(0.0f);
    for (auto b : flock) {
        if (b->id != id) {
            if (distance(pos, b->pos) < r1Dist + b->r) {
                repellant -= (b->pos - pos);
            }
        }
    }
    for (auto o : obstacles) {
        if (distance(pos, o->pos) < r1Dist + o->r) {
            repellant -= (o->pos - pos);
        }
    }
    return repellant;
}

vec3 Boid::rule1a() {
    // rule 1: separation
    // if within a certain distance of another boid, move away
    float r1Dist = r; // default 2 * r
    vec3 repellant(0.0f);
    for (auto b : flock) {
        if (b->id != id) {
            vec3 bPos = b->getPos();
            float dist = distance(pos, bPos) - b->r - r;
            if (dist < r1Dist) {
                //float repMag = r1Dist - dist;
                //repellant -= repMag * normalize(b->pos - pos);
                float collDot = dot(velocity, (bPos - pos));
                if (collDot < 0) {
                    vec3 collVel = velocity * dot(velocity, (bPos - pos));
                    repellant += collVel;
                }
            }
        }
    }
    for (auto o : obstacles) {
        float dist = distance(pos, o->pos) - o->r - r;
        if (dist < r1Dist) {
            //float repMag = r1Dist - dist;
            //repellant -= repMag * normalize(o->pos - pos);
            float collDot = dot(velocity, (o->pos - pos));
            if (collDot < 0) {
                vec3 collVel = velocity * dot(velocity, (o->pos - pos));
                repellant += collVel;
            }
        }
    }
    return repellant;
}

vec3 Boid::rule2() {
    // rule 2: alignment
    // try and match average velocity of flock
    float r2Factor = 1 / 80.0f; // default 1/80
    vec3 meanV(0.0f);
    int iCount = 0;
    for (auto b : flock) {
        if (b->id != id) {
            if (distance(pos, b->getPos()) < influenceDist) {
                meanV += b->getVelocity();
                iCount++;
            }
        }
    }
    if (iCount == 0) return vec3(0.0f);
    meanV = meanV / (float)(iCount);
    return (meanV - velocity) * r2Factor;
}

vec3 Boid::rule3() {
    // rule 3: cohesion
    // move towards center of mass of all boids
    float r3Factor = 1 / 100.0f; // default 1/100
    vec3 center(0.0f);
    int iCount = 0;
    for (auto b : flock) {
        if (b->id != id) {
            vec3 bPos = b->getPos();
            if (distance(pos, bPos) < influenceDist) {
                center += bPos;
                iCount++;
            }
        }
    }
    if (iCount == 0) return vec3(0.0f);
    center = center / (float)(iCount);
    return (center - pos) * r3Factor;
}

vec3 Boid::moveTarget() {
    // move towards target
    float targetFactor = 1 / 100.0f; // default 1/100
    if (targetEnabled) {
        return (target - pos) * targetFactor;
    }
    return vec3(0.0f);
}

vec3 Boid::moveTarget2() {
    // move towards target
    float targetFactor = 1 / 10.0f; // default 1/100
    if (targetEnabled) {
        return targetFactor * normalize(target - pos);
    }
    return vec3(0.0f);
}

vec3 Boid::stayBounds() {
    // stay in bounds
    float boundsFactor = 0.1f; // default 0.1
    vec3 b(0.0f);
    if (boundsSet) {
        if (pos.x < bounds.minX)
            b.x = boundsFactor * (bounds.minX - pos.x);
        else if (pos.x > bounds.maxX)
            b.x = boundsFactor * (bounds.maxX - pos.x);
        if (pos.y < bounds.minY)
            b.y = boundsFactor * (bounds.minY - pos.y);
        else if (pos.y > bounds.maxY)
            b.y = boundsFactor * (bounds.maxY - pos.y);
        if (pos.z < bounds.minZ)
            b.z = boundsFactor * (bounds.minZ - pos.z);
        else if (pos.z > bounds.maxZ)
            b.z = boundsFactor * (bounds.maxZ - pos.z);
    }
    return b;
}