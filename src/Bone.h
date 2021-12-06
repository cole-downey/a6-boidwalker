#ifndef _BONE_H_
#define _BONE_H_

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "MatrixStack.h"
#include "Program.h"

class Bone {
public:
    Bone();
    ~Bone() {};

    void addKeyframe(glm::vec3 pos, glm::quat rot);
    glm::mat4 getKeyframe(int frame);
    glm::mat4 getBind();
    glm::mat4 getInverseBind();
    void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, int frame = 0);
private:
    std::shared_ptr<std::vector<glm::mat4> > keyframes;
    bool bindSet;
    glm::mat4 bind;
    glm::mat4 inverseBind;
};

#endif