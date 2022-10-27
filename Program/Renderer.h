#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include "Util.h"
#include "Program.h"
#include "Object.h"
#include "Skybox.h"

class AbstractRenderer {
protected:
	Program* program;
	static GLuint VAO;
	GLuint texture_sampler_ID;
public:
	AbstractRenderer(Program* program);
	virtual void render() = 0;
};

// Renderer for 
class ObjectRenderer : public AbstractRenderer {
private:
	// One object can be displayed multiple times with different transformations
	std::map<object::Object*, std::vector<trans::Transformation*>> objects;
	glm::mat3 light;

	GLuint model_matrix_ID, light_ID, mesh_matrix_ID, material_ID;
public:
	ObjectRenderer(Program* program);
	void setLight(glm::mat3 light);

	void addObject(object::Object* obj, trans::Transformation* transformation);
	void addObject(object::Object* obj, std::vector<trans::Transformation*> transformations);

	void render();
};

// Renderer specialized for rendering skybox only
class SkyboxRenderer : public AbstractRenderer {
private:
	GLuint cube_VBO, texture_ID;
public:
	SkyboxRenderer(Program* program, GLuint texture_ID);
	void render();
};

#endif