#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#define RENDERERS std::tuple<std::vector<AbstractRenderer*>, std::vector<Renderer*>, std::vector<AbstractRenderer*>>

#include <map>
#include <tuple>

#include "Util.h"
#include "Program.h"
#include "Object.h"
#include "Light.h"
#include "Camera.h"

class AbstractRenderer {
protected:
	Program* program;
public:
	AbstractRenderer() {};
	AbstractRenderer(Program* program);
	~AbstractRenderer();
	virtual void render() = 0;
};


class Renderer {
private:
	Program* program;
	GLuint irradiance_ID, prefiltered_map_ID, brdf_ID;
	GLuint depth_map_ID, use_shadows_ID;
public:
	~Renderer();
	Renderer(Program* program);
	void prepare(int* transformations_idx, GLint depth_map_ID);
	void render(object::Mesh* mesh, size_t count);
};

/* Renderer specialized for rendering skybox only
class SkyboxRenderer : public AbstractRenderer {
private:
	GLuint cube_VBO;
	Texture* texture;
public:
	SkyboxRenderer(Program* program, Texture* texture);
	void render();
};
*/

class CrosshairRenderer : public AbstractRenderer, public WindowSizeSubscriber {
private:
	GLuint sampler_ID;
	GLuint VBO;
	GLuint aspect_ratio_ID;
public:
	CrosshairRenderer();
	void render();
	void updateSize(int width, int height);
};

class EnvMapRenderer : public AbstractRenderer {
private:
	GLuint sampler_ID;
	GLuint sphere_VBO;
	GLuint triangles;
public:
	EnvMapRenderer(std::vector<float> sphere, Camera* camera);
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