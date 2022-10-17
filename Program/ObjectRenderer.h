#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include "Util.h"
#include "Program.h"
#include "Object.h"

class ObjectRenderer {
private:
	// One object can be displayed multiple times with different transformations
	std::map<Object*, std::vector<trans::Transformation*>> objects;
	glm::mat3 light;

	Program* program;

	GLuint model_matrix_ID, light_ID, texture_sampler_ID;
	static GLuint VAO;
public:
	ObjectRenderer(Program* program);
	void setLight(glm::mat3 light);

	void addObject(Object* obj, trans::Transformation* transformation);
	void addObject(Object* obj, std::vector<trans::Transformation*> transformations);

	void render();
};

#endif