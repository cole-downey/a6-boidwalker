#include "BoneCharacter.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Bone.h"
#include "ShapeSkin.h"
#include "Program.h"
#include "MatrixStack.h"

using namespace std;
using namespace glm;

BoneCharacter::BoneCharacter() : fps(15), scale(0.015f) {
    bones = make_shared<vector<shared_ptr<Bone> > >();
    scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    walkInCircle = false;
    circleR = 2.0f;
    tCircle = (int)fps * 10.0f;
}

BoneCharacter::~BoneCharacter() {
    bones = make_shared<vector<shared_ptr<Bone> > >();
}

void BoneCharacter::loadAnimData(const string& dataDir) {
    DATA_DIR = dataDir;
    string filename = DATA_DIR + "input.txt";
    ifstream in;
    in.open(filename);
    if (!in.good()) {
        cout << "Cannot read " << filename << endl;
        return;
    }
    //cout << "Loading " << filename << endl;

    string line;
    while (1) {
        getline(in, line);
        if (in.eof()) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        // Skip comments
        if (line.at(0) == '#') {
            continue;
        }
        // Parse lines
        string key, value;
        stringstream ss(line);
        // key
        ss >> key;
        if (key.compare("TEXTURE") == 0) {
            ss >> value;
            //animInput.textureData.push_back(value);
        } else if (key.compare("MESH") == 0) {
            vector<string> mesh;
            ss >> value;
            mesh.push_back(value); // obj
            ss >> value;
            mesh.push_back(value); // skin
            animInput.meshData.push_back(mesh);
        } else if (key.compare("SKELETON") == 0) {
            ss >> value;
            animInput.skeletonData = value;
        } else {
            cout << "Unknown key word: " << key << endl;
        }
    }
    in.close();
}

void BoneCharacter::init() {
    initBones();
    initShapes();
    // calc the number of verts of each shape, and store them
    totalVert = 0;
    for (auto s : shapes) {
        totalVert += s->getNVerts();
        vertsPerShape.push_back(s->getNVerts());
    }
}

void BoneCharacter::initBones() {
    string filename = DATA_DIR + animInput.skeletonData;
    if (filename == DATA_DIR + "bigvegas_WalkingInPlace_skel.txt") walkInCircle = true;
    ifstream in;
    in.open(filename);
    if (!in.good()) {
        cout << "Cannot read " << filename << endl;
        return;
    }
    //cout << "Loading " << filename << endl;

    string line;
    while (1) {
        getline(in, line);
        if (in.eof()) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        // Skip comments
        if (line.at(0) == '#') {
            continue;
        }
        // number of frames and bones
        stringstream ss(line);
        ss >> nFrames;
        ss >> nBones;
        break;
    }
    // fill bones vector
    for (int i = 0; i < nBones; i++) {
        bones->push_back(make_shared<Bone>());
    }
    while (1) {
        getline(in, line);
        if (in.eof()) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        // Skip comments
        if (line.at(0) == '#') {
            continue;
        }
        // Parse lines
        float qx, qy, qz, qw, px, py, pz;
        stringstream ss(line);
        for (int i = 0; i < nBones; i++) {
            ss >> qx >> qy >> qz >> qw;
            ss >> px >> py >> pz;
            bones->at(i)->addKeyframe(glm::vec3(px, py, pz), glm::quat(qw, qx, qy, qz));
        }
    }
    in.close();
}

void BoneCharacter::initShapes() {
    for (const auto& mesh : animInput.meshData) {
        auto shape = make_shared<ShapeSkin>(bones);
        shapes.push_back(shape);
        shape->loadMesh(DATA_DIR + mesh[0]);
        shape->loadAttachment(DATA_DIR + mesh[1]);
        shape->init();
    }
}


int BoneCharacter::draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, double t) {
    // returns frame #
    frame = ((int)floor(t * fps)) % nFrames;
    frameTotal = (int)floor(t * fps);
    //draw(MV, prog, frame);
    return frame;
}

void BoneCharacter::draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, int _frame) {
    MV->pushMatrix();
    MV->scale(scale);
    for (auto b : *bones) {
        b->draw(MV, prog, _frame);
    }
    MV->popMatrix();
}

glm::vec3 BoneCharacter::getBonePos(int i, int f) {
    glm::mat4 E = bones->at(i)->getKeyframe(f);
    glm::vec4 p = scaleMat * E * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return glm::vec3(p);
}

glm::vec3 BoneCharacter::getVertPos(int i, int f, int fSum) {
    assert(i < totalVert);
    int shapeInd = 0;
    int vertsCounted = 0;
    for (int s = 0; s < shapes.size(); s++) {
        vertsCounted += vertsPerShape.at(s);
        if (vertsCounted - 1 >= i) {
            shapeInd = s;
            break;
        }
    }
    if (walkInCircle) {
        float fPart = (float)(fSum % tCircle) / tCircle;
        double t = 2 * 3.14159f * fPart;
        vec3 pos = vec3(circleR * (float)sin(t), 0.0f, circleR * (float)cos(t));
        auto posMat = glm::translate(glm::mat4(1.0f), pos);
        auto rotMat = glm::rotate(glm::mat4(1.0f), (float)t + radians(90.0f), vec3(0, 1, 0));
        glm::vec4 p = posMat * rotMat * scaleMat * glm::vec4(shapes.at(shapeInd)->getVertPos(i, f), 1.0f);
        return glm::vec3(p);
    }
    glm::vec4 p = scaleMat * glm::vec4(shapes.at(shapeInd)->getVertPos(i, f), 1.0f);
    return glm::vec3(p);
}