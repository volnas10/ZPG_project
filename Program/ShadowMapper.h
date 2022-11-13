#pragma once
#ifndef SHADOWMAPPER_H
#define SHADOWMAPPER_H

#include "Light.h"

class ShadowMapper {
private:
	GLuint depth_map_FBO;
	Program* shadow_program;
public:
	ShadowMapper();
	void renderShadows();
};

#endif

