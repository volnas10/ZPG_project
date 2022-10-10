#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include "Camera.h"
#include "Util.h"
#include "Program.h"
#include "Object.h"

class ObjectRenderer {
private:
	// One object can be displayed multiple times with different transformations
	std::map<Object*, std::vector<trans::Transformation*>> objects;

	Program* program;

	GLuint model_matrix_ID;
	static GLuint VAO;
public:
	ObjectRenderer(Program* program, Camera* camera);

	void addObject(Object* obj, trans::Transformation*);

	void render();
};

#endif