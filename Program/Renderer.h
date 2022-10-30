#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include "Util.h"
#include "Program.h"
#include "Object.h"

class AbstractRenderer {
protected:
	Program* program;
	static GLuint VAO;
	std::vector<GLuint> texture_samplers;
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

// Floor renderer able to tile textures
class FloorRenderer : public AbstractRenderer {
private:
	GLuint plane_VBO, offset_buffer, rotation_buffer;
	GLuint diffuse_tex_ID, normal_tex_ID, specular_tex_ID;
	int tile_count;
	glm::mat3 light;
public:
	FloorRenderer(Program* program, float size, int dimension, GLuint diffuse, GLuint normal, GLuint specular);
	void setLight(glm::mat3 light);
	void render();
};

#endif