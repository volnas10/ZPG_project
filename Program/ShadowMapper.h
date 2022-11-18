#pragma once
#ifndef SHADOWMAPPER_H
#define SHADOWMAPPER_H

#include "Light.h"
#include "Object.h"
#include "Observers.h"

class ShadowMapper : public LightSubscriber{
private:
	GLuint depth_map_FBO;
	Program* shadow_program;
public:
	ShadowMapper();
	void renderShadows(object::Mesh* mesh);
	void updateLights(std::vector<Light*> lights);
};

#endif

