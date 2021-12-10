#ifndef BoneCharacter_H
#define BoneCharacter_H

#include <string>
#include <vector>
#include <memory>

class Bone;
class ShapeSkin;
class Program;
class MatrixStack;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class BoneCharacter {
public:
    // Stores information in data/input.txt
    struct AnimDataInput {
        std::vector< std::vector<std::string> > meshData;
        std::string skeletonData;
    };

    BoneCharacter();
    ~BoneCharacter();
    void loadAnimData(const std::string& dataDir);
    void init();
    void initBones();
    void initShapes();
    int draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, double t);
    void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, int _frame);
    int getFrame() { return frame; };
    glm::vec3 getBonePos(int i, int f);
    glm::vec3 getVertPos(int i, int f, int fSum = 0);
    int getNTargets() { return nBones; };
    int getNTargetsVert() { return totalVert; };

    // walking in circle
    bool walkInCircle = false;
    float circleR;
    int tCircle;
    int frameTotal;
private:
    int frame;
    AnimDataInput animInput;
    std::vector< std::shared_ptr<ShapeSkin> > shapes;
    std::shared_ptr<std::vector<std::shared_ptr<Bone> > > bones;
    int nFrames, nBones;
    std::string DATA_DIR;
    double fps;
    float scale;
    glm::mat4 scaleMat;

    // vertex calc
    int totalVert;
    std::vector<int> vertsPerShape;
};

#endif