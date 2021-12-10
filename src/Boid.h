#pragma once
#ifndef Boid_H
#define Boid_H

#include <vector>
#include <memory>
#include <mutex>
#include "Semaphore.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Bounds.h"

class Shape;
class Program;
class MatrixStack;
class Obstacle;

class Boid {
public:
    Boid(glm::vec3 _pos);
    ~Boid() {};
    void update(float timestep);
    void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> p) const;
    void setFlock(const std::vector< std::shared_ptr<Boid> >& _flock) { flock = _flock; };
    void setObstacles(const std::vector< std::shared_ptr<Obstacle> >& _obs) { obstacles = _obs; };
    void setShape(const std::shared_ptr<Shape> _shape) { shape = _shape; };
    void setTarget(glm::vec3 _target) { target = _target; targetSet = true; };
    void toggleTarget() { if (targetSet) targetEnabled = !targetEnabled; };
    void setBounds(Bounds _bounds) { bounds = _bounds; boundsSet = true; };
    glm::vec3 getVelocity();
    glm::vec3 getPos();

    static int ID_COUNT;
    int id;
    int targetId;
    float r;
private:
    glm::vec3 pos;
    glm::vec3 velocity;

    glm::vec3 rule1(); // separation
    glm::vec3 rule2(); // alignment
    glm::vec3 rule3(); // cohesion
    glm::vec3 moveTarget();
    glm::vec3 moveTarget2();
    glm::vec3 stayBounds();

    std::vector< std::shared_ptr<Boid> > flock;
    std::vector< std::shared_ptr<Obstacle> > obstacles;
    std::shared_ptr<Shape> shape;

    glm::vec3 target;
    bool targetSet = false;
    bool targetEnabled = false;
    bool targetBlocked = false;

    Bounds bounds;
    bool boundsSet = false;

    float influenceDist; // how close does another boid have to be to influence the rest

    // threading
    std::mutex mtxP, mtxV;
};

#endif