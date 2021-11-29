#include <iostream>
#include <random>
#include <cmath>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scene.h"
#include "Shape.h"
#include "Program.h"
#include "Boid.h"
#include "Bounds.h"
#include "Obstacle.h"


using namespace std;
using namespace glm;

Scene::Scene() :
	t(0.0),
	h(1e-2) {
}

Scene::~Scene() {
}

void Scene::load(const string& RESOURCE_DIR) {
	// Units: meters, kilograms, seconds
	h = 5e-3 * 5;


	boidShape = make_shared<Shape>();
	boidShape->loadMesh(RESOURCE_DIR + "cone.obj");
	obsShape = make_shared<Shape>();
	obsShape->loadMesh(RESOURCE_DIR + "sphere2.obj");

	auto obs = make_shared<Obstacle>(vec3(1.0f, 1.0f, 1.0f), 0.35f);
	obs->setShape(obsShape);
	obstacles.push_back(obs);
	obs = make_shared<Obstacle>(vec3(-1.0f, 1.0f, -1.0f), 0.75f);
	obs->setShape(obsShape);
	obstacles.push_back(obs);

	int nBoids = 600;
	auto bounds = Bounds(4.0f, 3.0f, 4.0f, -4.0f, 0.0f, -4.0f);
	auto target1 = vec3(0.0f, 0.5f, 0.0f);
	auto target2 = vec3(-2.5f, 0.5f, 1.5f);
	auto target3 = vec3(2.5f, 0.5f, 1.5f);
	vec3 targetIncrement = (target3 - target2) / (float)(nBoids / 2);
	for (int i = 0; i < nBoids; i++) {
		auto boid = make_shared<Boid>();
		boid->pos = generateRandomPos2(bounds);
		boid->setShape(boidShape);
		boid->setFlock(boidFlock);
		boid->setObstacles(obstacles);
		boid->setBounds(bounds);
		//boid->setTarget(target2 + targetIncrement * (float)i);
		if (i % 2 == 0) boid->setTarget(target2 + targetIncrement * (float)(i / 2));
		//else boid->setTarget(target2);
		boidFlock.push_back(boid);
	}
	for (auto b : boidFlock) {
		b->setFlock(boidFlock);
	}

}

void Scene::init() {
	boidShape->init();
	boidShape->normalize();
	obsShape->init();
	obsShape->normalize();
}

void Scene::tare() {
}

void Scene::reset() {
	t = 0.0;
}

void Scene::step() {
	t += h;
	for (auto b : boidFlock) {
		b->update(h);
	}
}

void Scene::step(double timestep) {
	h = timestep;
	step();
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

void Scene::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> prog) const {
	glUniform3fv(prog->getUniform("kdFront"), 1, value_ptr(vec3(1.0f)));
	for (auto b : boidFlock) {
		b->draw(MV, prog);
	}
	for (auto o : obstacles) {
		o->draw(MV, prog);
	}
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
