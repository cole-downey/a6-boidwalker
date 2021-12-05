#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Obstacle.h"
#include "Shape.h"
#include "Program.h"
#include "MatrixStack.h"

using namespace std;
using namespace glm;

Obstacle::Obstacle() :
    pos(0.0f),
    r(0.0f) {
}

Obstacle::Obstacle(vec3 _pos, float _r) :
    pos(_pos),
    r(_r) {
}

void Obstacle::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> prog) const {
    if (shape) {
        glUniform3fv(prog->getUniform("kdFront"), 1, glm::value_ptr(vec3(1.0)));
        MV->pushMatrix();
        if (!pointing) MV->translate(pos);
        if (pointing) {
            glm::mat4 E = inverse(lookAt(pos, vec3(0.0f), vec3(0, 1, 0)));
            MV->multMatrix(E);
        }
        MV->scale(r);
        if (pointing) MV->rotate(radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        shape->draw(prog);
        MV->popMatrix();
    }
}