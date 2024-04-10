#pragma once
#ifndef RENDERINGSCHEDULER_H
#define RENDERINGSCHEDULER_H

#include <array>
#include "Object.h"
#include "Renderer.h"
#include "TransformationBuffer.h"
#include "ShadowMapper.h"

struct MeshInstances {
	object::Mesh* mesh;
	TransformationBuffer* instances;
};

class RenderingScheduler : public LightSubscriber {
private:
	GLuint VAO, depth_FBO;
	std::vector<MeshInstances> meshes;
	std::vector<Light::LightStruct> lights;
	std::vector<AbstractRenderer*> pre_renderers;
	std::vector<Renderer*> main_renderers;
	std::vector<AbstractRenderer*> post_renderers;
	ShadowMapper* shadow_mapper;
	unsigned int shadow_type;

	DepthMapRenderer* depth_map_renderer;

public:
	RenderingScheduler(object::ObjectGroup* group, RENDERERS renderers);
	void updateLights(std::vector<Light::LightStruct> lights);
	void render(float viewport_width, float viewport_height);
	void setShadowType(unsigned int type);

	void addObjectAtRuntime(trans::Transformation* transformation);

	float depthAtPos(int width, int height);

	ShadowMapper* getShadowMapper();
};

#endif
