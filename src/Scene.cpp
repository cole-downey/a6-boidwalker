#include <iostream>
#include <random>
#include <cmath>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef _GLIBCXX_USE_NANOSLEEP
#define _GLIBCXX_USE_NANOSLEEP
#endif
#include <thread>

#include "Scene.h"
#include "Shape.h"
#include "Program.h"
#include "Boid.h"
#include "Bounds.h"
#include "Obstacle.h"
#include "PCBuffer.h"
#include "BoneCharacter.h"


using namespace std;
using namespace glm;

Scene::Scene() :
	h(1e-3),
	useThreads(true),
	nThreads(8),
	nBoids(600) {
}

Scene::~Scene() {
	// join threads
	if (useThreads) {
		for (int i = 0; i < threads.size(); i++) {
			//for (auto t : threads) {
			threads.at(i).detach();
		}
	}
	delete commandPCB;
	delete responsePCB;
}

void Scene::load(const string& RESOURCE_DIR, const string& DATA_DIR, bool useObstacles) {
	// Units: meters, kilograms, seconds
	boidShape = make_shared<Shape>();
	boidShape->loadMesh(RESOURCE_DIR + "cone.obj");
	obsShape = make_shared<Shape>();
	obsShape->loadMesh(RESOURCE_DIR + "sphere2.obj");

	if (useObstacles) {
		auto obs = make_shared<Obstacle>(vec3(0.0f, 1.5f, -2.0f), 1.0f);
		obs->setShape(obsShape);
		obstacles.push_back(obs);
		obs = make_shared<Obstacle>(vec3(-1.0f, 1.5f, 1.0f), 0.75f);
		obs->setShape(obsShape);
		//obstacles.push_back(obs);
	}

	// bone character
	boneCharacter = make_shared<BoneCharacter>();
	boneCharacter->loadAnimData(DATA_DIR);
	boneCharacter->init();

	// boid flock
	int nTargets = boneCharacter->getNTargetsVert();
	auto bounds = Bounds(4.0f, 3.0f, 4.0f, -4.0f, 0.0f, -4.0f);
	auto target1 = vec3(0.0f, 0.5f, 0.0f);
	auto target2 = vec3(-2.5f, 0.5f, 1.5f);
	auto target3 = vec3(2.5f, 0.5f, 1.5f);
	vec3 targetIncrement = (target3 - target2) / (float)(nBoids / 2);
	for (int i = 0; i < nBoids; i++) {
		auto boid = make_shared<Boid>(generateRandomPos2(bounds));
		boid->setShape(boidShape);
		boid->setFlock(boidFlock);
		boid->setObstacles(obstacles);
		boid->setBounds(bounds);
		//boid->setTarget(target2 + targetIncrement * (float)i);
		//if (i % 2 == 0) boid->setTarget(target2 + targetIncrement * (float)(i / 2));
		//if (i % 2 == 0) boid->setTarget(target1);
		boid->targetId = (nTargets < nBoids) ? i % nTargets : (nTargets / nBoids) * i;
		boidFlock.push_back(boid);
	}
	for (auto b : boidFlock) {
		b->setFlock(boidFlock);
	}

	cout << "scene load done" << endl;
}

void Scene::init() {
	boidShape->init();
	boidShape->normalize();
	obsShape->init();
	obsShape->normalize();

	// threading
	if (useThreads) {
		commandPCB = new PCBuffer<float>(10);
		responsePCB = new PCBuffer<bool>(10);

		int boidsPerThread = nBoids / nThreads;
		for (int i = 0; i < nThreads; i++) {
			int first = i * boidsPerThread; // first i for a thread to use
			int last = (i + 1) * boidsPerThread - 1; // last i, inclusive
			if (i == nThreads - 1) last = nBoids - 1;
			threads.push_back(thread(&Scene::boidHandler, this, i, first, last));
		}
	}
}

void Scene::threadTest() {
	if (useThreads) {
		for (int i = 0; i < nThreads; i++) {
			commandPCB->deposit(-1.0f);
			responsePCB->retrieve();
		}
	}
}

void Scene::step() {
	if (frame != prevFrame) { // only update targets if frame changes
		updateTargets();
		prevFrame = frame;
	}
	if (useThreads) {
		for (int i = 0; i < threads.size(); i++) {
			// deposit h once for each thread
			commandPCB->deposit((float)h);
		}
		for (int i = 0; i < threads.size(); i++) {
			// wait for each thread to complete before returning
			responsePCB->retrieve();
		}
	} else {
		for (auto b : boidFlock) {
			b->update((float)h);
		}
	}
}

void Scene::step(double timestep) {
	h = timestep;
	step();
}

void Scene::boidHandler(int pid, int first, int last) {
	//cout << "thread " << pid << " created" << endl;
	while (true) {
		float command = commandPCB->retrieve();
		if (command < 0) {
			cout << "command ignore" << endl;
		} else {
			// for each boid, do command
			for (int i = first; i <= last; i++) {
				boidFlock.at(i)->update(command);
			}
		}
		responsePCB->deposit(true);
		this_thread::sleep_for(chrono::microseconds(1));
	}
}

void Scene::toggleTarget() {
	for (auto b : boidFlock) {
		b->toggleTarget();
	}
}

void Scene::togglePointingObs() {
	for (auto o : obstacles) {
		o->pointing = !o->pointing;
	}
}

void Scene::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> prog, double animt) const {
	glUniform3fv(prog->getUniform("kdFront"), 1, value_ptr(vec3(1.0f)));
	for (auto b : boidFlock) {
		b->draw(MV, prog);
	}
	for (auto o : obstacles) {
		o->draw(MV, prog);
	}
	frame = boneCharacter->draw(MV, prog, animt);
	frameSum = boneCharacter->frameTotal;
}

vec3 Scene::generateRandomPos(Bounds bounds) {
	vec3 pos;
	float rMult = 1.0f;
	float rangeX = bounds.maxX - bounds.minX;
	float rangeY = bounds.maxY - bounds.minY;
	float rangeZ = bounds.maxZ - bounds.minZ;
	pos.x = (float)rand() / (float)(RAND_MAX / rangeX) - rangeX / 2;
	pos.y = (float)rand() / (float)(RAND_MAX / rangeY) - rangeY / 2;
	pos.z = (float)rand() / (float)(RAND_MAX / rangeZ) - rangeZ / 2;
	if (pos.x > 0) pos.x += bounds.maxX;
	else  pos.x += bounds.minX;
	if (pos.y > 0) pos.y += bounds.maxY;
	else  pos.y += bounds.minY;
	if (pos.z > 0) pos.z += bounds.maxZ;
	else  pos.z += bounds.minZ;
	return pos;
}

vec3 Scene::generateRandomPos2(Bounds bounds) {
	vec3 center(0.0f);
	vec3 pos;
	float rMult = 4.0f;
	float rangeX = bounds.maxX - bounds.minX;
	float rangeY = bounds.maxY - bounds.minY;
	float rangeZ = bounds.maxZ - bounds.minZ;
	float spawnRadius = sqrt(rangeX * rangeX + rangeY * rangeY + rangeZ * rangeZ) / 2;
	pos.x = bounds.minX * rMult + (float)rand() / (float)(RAND_MAX / (rangeX * rMult));
	pos.y = bounds.minY * rMult + (float)rand() / (float)(RAND_MAX / (rangeY * rMult));
	pos.z = bounds.minZ * rMult + (float)rand() / (float)(RAND_MAX / (rangeZ * rMult));
	vec3 r = pos - center;
	pos += spawnRadius * normalize(r);
	return pos;
}

void Scene::updateTargets() {
	for (auto b : boidFlock) {
		b->setTarget(boneCharacter->getVertPos(b->targetId, frame, frameSum));
	}
}

