#ifndef BoneCharacter_H
#define BoneCharacter_H

#include <string>
#include <vector>
#include <memory>

class Bone;
class ShapeSkin;
class Program;
class MatrixStack;


class BoneCharacter {
public:
    // Stores information in data/input.txt
    struct AnimDataInput {
        std::vector< std::vector<std::string> > meshData;
        std::string skeletonData;
    };

    BoneCharacter();
    ~BoneCharacter();
    void loadAnimData(std::string dataDir);
    void init();
    void initBones();
    void initShapes();
    void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, double t);
    void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, int frame);
private:
    AnimDataInput animInput;
    std::vector< std::shared_ptr<ShapeSkin> > shapes;
    std::shared_ptr<std::vector<Bone> > bones;
    int nFrames, nBones;
    std::string DATA_DIR;
};

#endif