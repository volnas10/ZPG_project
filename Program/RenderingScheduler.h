#pragma once
#ifndef RENDERINGSCHEDULER_H
#define RENDERINGSCHEDULER_H

#include "Object.h"
#include "Renderer.h"
#include "TransformationBuffer.h"

struct MeshInstances {
	object::Mesh* mesh;
	std::map<Renderer*, TransformationBuffer*> instances;
};

class RenderingScheduler {
private:
	GLuint VAO;
	std::vector<MeshInstances> meshes;
	LightCollection* lights;
	std::vector<AbstractRenderer*> other_renderers;
	//ShadowMapper shadow_mapper;
public:
	RenderingScheduler();
	void addOtherRenderers(std::vector<AbstractRenderer*> renderers);
	void addRenderingGroups(std::vector<object::Object*> objects, std::vector<RenderingGroup*> groups);
	void setLights(LightCollection* lights);
	void render();
};

#endif
