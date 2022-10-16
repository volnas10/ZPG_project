#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include "Camera.h"
#include "Util.h"
#include "Program.h"
#include "Object.h"
#include "Scene.h"

class ObjectRenderer {
private:
	// One object can be displayed multiple times with different transformations
	std::map<Object*, std::vector<trans::Transformation*>> objects;


	Program* program;

	GLuint model_matrix_ID, light_ID, texture_sampler_ID;
	static GLuint VAO;
public:
	ObjectRenderer(Program* program);

	void addObject(Object* obj, trans::Transformation*);

	void loadFromScene(Scene* scene);

	void render();
};

#endif