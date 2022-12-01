#pragma once
#ifndef RENDERINGSCHEDULER_H
#define RENDERINGSCHEDULER_H

#include "Object.h"
#include "Renderer.h"
#include "TransformationBuffer.h"
#include "ShadowMapper.h"

struct MeshInstances {
	object::Mesh* mesh;
	std::map<int, std::pair<Renderer*, TransformationBuffer*>> instances;
};

class RenderingScheduler : public LightSubscriber {
private:
	GLuint VAO, depth_FBO;
	std::vector<MeshInstances> meshes;
	std::vector<Light::LightStruct> lights;
	std::vector<AbstractRenderer*> pre_renderers;
	std::vector<AbstractRenderer*> post_renderers;
	ShadowMapper* shadow_mapper;
	bool use_shadows;

	DepthMapRenderer* depth_map_renderer;
	int selected_object_id;

public:
	RenderingScheduler();
	void addPreRenderer(AbstractRenderer* renderer);
	void addPostRenderer(AbstractRenderer* renderer);
	void addRenderingGroups(std::vector<object::Object*> objects, std::vector<RenderingGroup*> groups);
	void updateLights(std::vector<Light::LightStruct> lights);
	void render(float viewport_width, float viewport_height);
	void useShadows();

	void addObjectAtRuntime(trans::Transformation* transformation);

	void selectObject(int width, int height);
	float depthAtPos(int width, int height);

	ShadowMapper* getShadowMapper();
};

#endif
