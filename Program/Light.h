#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

class Light {
private:
	enum Type;
	Type light_type;
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 attenuation;
	float angle;
public:
	static enum Type{POINT = 0, DIRECTIONAL = 1, SPOTLIGHT = 2, FLASHLIGHT = 3};
	struct LightStruct {
		glm::vec4 color; // vec3 color + float power
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 attenuation; // constant, linear, quadratic
		float angle;
		unsigned int type;
		int padding[46];
	};
	
	Light(glm::vec3 color);
	void makePoint(glm::vec3 position, glm::vec3 attenuation);
	void makeDirectional(glm::vec3 direction);
	void makeSpotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float angle);
	void makeFlashlight(glm::vec3 attenuation, float angle);

	LightStruct toStruct();
};

#endif