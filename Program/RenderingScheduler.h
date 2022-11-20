#pragma once
#ifndef RENDERINGSCHEDULER_H
#define RENDERINGSCHEDULER_H

#include "Object.h"
#include "Renderer.h"
#include "TransformationBuffer.h"
#include "ShadowMapper.h"

struct MeshInstances {
	object::Mesh* mesh;
	std::map<Renderer*, TransformationBuffer*> instances;
};

class RenderingScheduler : public LightSubscriber {
private:
	GLuint VAO;
	std::vector<MeshInstances> meshes;
	std::vector<Light::LightStruct> lights;
	std::vector<AbstractRenderer*> other_renderers;
	ShadowMapper* shadow_mapper;
	bool use_shadows;

	DepthMapRenderer* depth_map_renderer;
public:
	RenderingScheduler();
	void addOtherRenderers(std::vector<AbstractRenderer*> renderers);
	void addRenderingGroups(std::vector<object::Object*> objects, std::vector<RenderingGroup*> groups);
	void updateLights(std::vector<Light::LightStruct> lights);
	void render(float viewport_width, float viewport_height);
	void useShadows();

	ShadowMapper* getShadowMapper();
};

#endif
