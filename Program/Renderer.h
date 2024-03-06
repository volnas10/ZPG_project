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
	GLuint irradiance_ID, diffuse_ID, normal_ID, opacity_ID, depth_map_ID, has_textures_ID, use_shadows_ID;
public:
	Renderer(Program* program);
	void prepare(int* transformations_idx, GLint depth_map_ID);
	void render(object::Mesh* mesh, size_t count);
};

class RenderingGroup {
private:
	static int id_counter;
	Renderer* renderer;
	std::map<object::Object*, std::pair<trans::Transformation*, std::vector<trans::Transformation*>>> objects;
public:
	int id;

	RenderingGroup(Program* program);
	void addObjectTransformation(object::Object* object, trans::Transformation* transformation);
	void addAllObjectTransformations(object::Object* object, std::pair<trans::Transformation*, std::vector<trans::Transformation*>> transformations);
	std::vector<trans::Transformation*> getTransformations(object::Object* object);
	trans::Transformation* getDefaultTransformation(object::Object* object);
	Renderer* getRenderer();
};

class AbstractRenderer {
protected:
	Program* program;
	std::vector<GLuint> texture_samplers;
public:
	AbstractRenderer() {};
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

class CrosshairRenderer : public AbstractRenderer, public WindowSizeSubscriber {
private:
	GLuint VBO, aspect_ratio_ID;
	Texture* crosshair_texture;
public:
	CrosshairRenderer();
	void render();
	void updateSize(int width, int height);
};

class EnvMapRenderer : public AbstractRenderer {
private:
	GLuint sphere_VBO;
	GLuint triangles;
	Texture* texture;
public:
	EnvMapRenderer(Program* program, Texture* texture, std::vector<float> sphere);
	void render();
};

// Used for debugging
class DepthMapRenderer {
private:
	Program* program;
	GLuint sampler_ID;
	GLuint VBO;
public:
	DepthMapRenderer();
	void render(GLuint depth_map_ID);
};

#endif