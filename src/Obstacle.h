#pragma once
#ifndef Obstacle_H
#define Obstacle_H

#include <vector>
#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class Shape;
class Program;
class MatrixStack;

class Obstacle {
public:
    Obstacle();
    Obstacle(glm::vec3 _pos, float _r);
    void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> p) const;
    void setShape(const std::shared_ptr<Shape> _shape) { shape = _shape; };

    glm::vec3 pos;
    float r;
    std::shared_ptr<Shape> shape;
    bool pointing = false;
};

#endif