#pragma once
#ifndef SHAPESKIN_H
#define SHAPESKIN_H

#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>

#include "Bone.h"

class MatrixStack;

class ShapeSkin {
public:
	struct Attachment {
		int vertInd;
		int boneInd;
		float weight;
		Attachment(int v, int b, float w) : vertInd(v), boneInd(b), weight(w) {};
	};

	ShapeSkin(std::shared_ptr<std::vector<std::shared_ptr<Bone> > > _bones);
	virtual ~ShapeSkin();
	void loadMesh(const std::string& meshName);
	void loadAttachment(const std::string& filename);
	void init();
	void update(int k);
	glm::vec3 getVertPos(int i);
	glm::vec3 getVertPos(int i, int f);
	int getNVerts() { return posBuf.size() / 3; };

private:
	std::vector<unsigned int> elemBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<float> bindPosBuf;
	std::vector<float> bindNorBuf;
	std::vector<std::vector<int> > attachBuf; // stores a list of attachments per vertex

	std::shared_ptr<std::vector<std::shared_ptr<Bone> > > bones;
	int nBones;
	std::shared_ptr<std::vector<Attachment> >weights;
};

#endif
