#include "Bone.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std;

Bone::Bone() {
    keyframes = make_shared<vector<glm::mat4> >();
    bindSet = false;
}

void Bone::addKeyframe(glm::vec3 pos, glm::quat rot) {
    glm::mat4 M = glm::mat4_cast(rot);
    M[3] = glm::vec4(pos, 1.0f);
    if (!bindSet) {
        bind = M;
        inverseBind = glm::inverse(M);
        bindSet = true;
    } else {
        keyframes->push_back(M);
    }
}

glm::mat4 Bone::getKeyframe(int frame) {
    return keyframes->at(frame);
}

glm::mat4 Bone::getBind() {
    return bind;
}
glm::mat4 Bone::getInverseBind() {
    return inverseBind;
}

void Bone::draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, int frame) {
    double s = 4.0;
    MV->pushMatrix(); {
        MV->multMatrix(keyframes->at(frame));
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

        glBegin(GL_LINES);
        // X in Red
        glColor3d(1.0, 0.0, 0.0);
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(s, 0.0, 0.0);
        // Y in Green
        glColor3d(0.0, 1.0, 0.0);
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(0.0, s, 0.0);
        // Z in Blue
        glColor3d(0.0, 0.0, 1.0);
        glVertex3d(0.0, 0.0, 0.0);
        glVertex3d(0.0, 0.0, s);
        glEnd();
    } MV->popMatrix();
}