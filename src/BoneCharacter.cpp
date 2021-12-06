#include "BoneCharacter.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "Bone.h"
#include "ShapeSkin.h"
#include "Program.h"
#include "MatrixStack.h"

using namespace std;

BoneCharacter::BoneCharacter() : fps(15), scale(0.015f) {
    bones = make_shared<vector<shared_ptr<Bone> > >();
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
}

void BoneCharacter::initBones() {
    string filename = DATA_DIR + animInput.skeletonData;
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
    }
}


int BoneCharacter::draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, double t) {
    // returns frame #
    frame = ((int)floor(t * fps)) % nFrames;
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
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    glm::vec4 p = S * E * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return glm::vec3(p);
}