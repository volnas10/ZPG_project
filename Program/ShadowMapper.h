#pragma once
#ifndef SHADOWMAPPER_H
#define SHADOWMAPPER_H

#include "Light.h"
#include "Object.h"
#include "Observers.h"

class ShadowMapper {
private:
	unsigned int shadow_type;
	GLuint depth_map, depth_map_FBO;
	GLuint texture_unit;
	Program* shadow_program;


	GLuint pvmatrix_ID, texture_ID, opacity_ID, has_textures_ID;
public:
	ShadowMapper(unsigned int type);
	void prepare(int* transformation_idx);
	void useLight(Light::LightStruct light, unsigned int index);
	void renderShadows(object::Mesh* mesh, size_t count);
	GLuint getUnit();

	GLuint getDepthMap();
};

#endif

