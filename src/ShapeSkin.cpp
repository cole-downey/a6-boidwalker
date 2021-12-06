#include "ShapeSkin.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "GLSL.h"
#include "Program.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "tiny_obj_loader.h"

using namespace std;
using namespace glm;

ShapeSkin::ShapeSkin(std::shared_ptr<std::vector<Bone> > _bones) :
	bones(_bones) {
	weights = make_shared<vector<Attachment> >();
}

ShapeSkin::~ShapeSkin() {
}

void ShapeSkin::loadMesh(const string& meshName) {
	// Load geometry
	// This works only if the OBJ file has the same indices for v/n/t.
	// In other words, the 'f' lines must look like:
	// f 70/70/70 41/41/41 67/67/67
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	const char* filename = meshName.c_str();

	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if (!rc) {
		cerr << errStr << endl;
	} else {
		posBuf = attrib.vertices;
		norBuf = attrib.normals;
		texBuf = attrib.texcoords;
		assert(posBuf.size() == norBuf.size());
		// bind pose buffers
		bindPosBuf = attrib.vertices;
		bindNorBuf = attrib.normals;
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			const tinyobj::mesh_t& mesh = shapes[s].mesh;
			size_t index_offset = 0;
			for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
				size_t fv = mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = mesh.indices[index_offset + v];
					elemBuf.push_back(idx.vertex_index);
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void ShapeSkin::loadAttachment(const std::string& filename) {
	ifstream in;
	in.open(filename);
	if (!in.good()) {
		cout << "Cannot read " << filename << endl;
		return;
	}
	cout << "Loading " << filename << endl;

	string line;
	int nVerts, maxNInfluences;
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
		// number of verts and influences
		stringstream ss(line);
		ss >> nVerts >> nBones >> maxNInfluences;
		break;
	}
	int vertInd = 0;
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
		int nWeights;
		stringstream ss(line);
		ss >> nWeights;
		for (int w = 0; w < nWeights; w++) {
			int boneInd;
			float weight;
			ss >> boneInd >> weight;
			weights->push_back(Attachment(vertInd, boneInd, weight));
		}
		vertInd++;
	}
	in.close();
}

void ShapeSkin::update(int k) {
	// clear the buffers
	for (int i = 0; i < posBuf.size(); i++) {
		posBuf.at(i) = 0;
		norBuf.at(i) = 0;
	}
	auto mBoneTransforms = vector<glm::mat4>();
	for (auto bone : *bones) {
		glm::mat4 MkM0i = bone.getKeyframe(k) * bone.getInverseBind();
		mBoneTransforms.push_back(MkM0i);
	}
	for (auto w : *weights) {
		glm::vec4 bindPos, bindNor;
		int vertInd = w.vertInd;
		bindPos = glm::vec4(bindPosBuf[vertInd * 3], bindPosBuf[vertInd * 3 + 1], bindPosBuf[vertInd * 3 + 2], 1.0);
		bindNor = glm::vec4(bindNorBuf[vertInd * 3], bindNorBuf[vertInd * 3 + 1], bindNorBuf[vertInd * 3 + 2], 0.0);
		glm::vec4 newPos, newNor;
		newPos = w.weight * mBoneTransforms.at(w.boneInd) * bindPos;
		newNor = w.weight * mBoneTransforms.at(w.boneInd) * bindNor;
		posBuf.at(vertInd * 3) += newPos.x;
		posBuf.at(vertInd * 3 + 1) += newPos.y;
		posBuf.at(vertInd * 3 + 2) += newPos.z;
		norBuf.at(vertInd * 3) += newNor.x;
		norBuf.at(vertInd * 3 + 1) += newNor.y;
		norBuf.at(vertInd * 3 + 2) += newNor.z;
	}
}

glm::vec3 ShapeSkin::getVertPos(int i) {
	return vec3(posBuf.at(i * 3), posBuf.at(i * 3 + 1), posBuf.at(i * 3 + 2));
}