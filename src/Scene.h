#pragma once
#ifndef Scene_H
#define Scene_H

#include <vector>
#include <memory>
#include <string>
#include <thread>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;
class Program;
class Shape;
class Boid;
class Obstacle;
struct Bounds;
#include "PCBuffer.h"

class Scene {
public:

	Scene();
	virtual ~Scene();

	void load(const std::string& RESOURCE_DIR);
	void init();
	void step();
	void step(double timestep);
	void toggleTarget();
	void togglePointingObs();

	void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> prog) const;

	void boidHandler(int pid, int first, int last); // thread function

	void threadTest();

private:
	// time
	double h;

	// objects
	int nBoids;
	std::shared_ptr<Shape> boidShape;
	std::shared_ptr<Shape> obsShape;
	std::vector< std::shared_ptr<Boid> > boidFlock;
	std::vector< std::shared_ptr<Obstacle> > obstacles;

	glm::vec3 generateRandomPos(Bounds bounds);
	glm::vec3 generateRandomPos2(Bounds bounds);

	// threading
	bool useThreads = false;
	int nThreads;
	PCBuffer<float>* commandPCB;
	PCBuffer<bool>* responsePCB;
	std::vector<std::thread> threads;

};

#endif
