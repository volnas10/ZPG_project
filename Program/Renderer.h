#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include "Util.h"
#include "Program.h"
#include "Object.h"
#include "Light.h"

// Right now standalone renderer, later might inherit from abstract renderer
class Renderer {
private:
	Program* program;
	GLuint diffuse_ID, normal_ID, opacity_ID, has_textures_ID;
public:
	Renderer(Program* program);
	void prepare(int* transformations_idx);
	void render(object::Mesh* mesh, size_t count);
};

class RenderingGroup {
private:
	Renderer* renderer;
	std::map<object::Object*, std::vector<trans::Transformation*>> objects;
public:
	RenderingGroup(Program* program);
	void addObjectTransformation(object::Object* object, trans::Transformation* transformation);
	void addAllObjectTransformations(object::Object* object, std::vector<trans::Transformation*> transformations);
	std::vector<trans::Transformation*> getTransformations(object::Object* object);
	Renderer* getRenderer();
};

class AbstractRenderer {
protected:
	Program* program;
	std::vector<GLuint> texture_samplers;
public:
	AbstractRenderer(Program* program);
	virtual void render() = 0;
};

// Renderer specialized for rendering skybox only
class SkyboxRenderer : public AbstractRenderer {
private:
	GLuint cube_VBO;
	Texture* texture;
public:
	SkyboxRenderer(Program* program, Texture* texture);
	void render();
};

// Floor renderer able to tile textures
class FloorRenderer : public AbstractRenderer {
private:
	GLuint plane_VBO, offset_buffer, rotation_buffer;
	int tile_count;
	Texture* texture;
public:
	FloorRenderer(Program* program, float size, int dimension, Texture* texture);
	void render();
};

#endif