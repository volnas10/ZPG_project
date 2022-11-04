#include "Light.h"

Light::Light(glm::vec3 color) {
	this->color = color;
}

void Light::makePoint(glm::vec3 position, glm::vec3 attenuation) {
	this->position = position;
	this->attenuation = attenuation;
	this->light_type = POINT;
}

void Light::makeDirectional(glm::vec3 direction) {
	this->direction = direction;
	this->light_type = DIRECTIONAL;
}

void Light::makeSpotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float angle) {
	this->position = position;
	this->direction = direction;
	this->attenuation = attenuation;
	this->angle = angle;
	this->light_type = SPOTLIGHT;
}

void Light::makeFlashlight(glm::vec3 attenuation, float angle) {
	this->attenuation = attenuation;
	this->angle = angle;
	this->light_type = FLASHLIGHT;
}

Light::LightStruct Light::toStruct() {
	LightStruct l_struc;
	l_struc.type = light_type;
	l_struc.color = glm::vec4(color, 0);

	if (light_type == POINT) {
		l_struc.position = glm::vec4(position, 0);
		l_struc.attenuation = glm::vec4(attenuation, 0);
	}
	else if (light_type == DIRECTIONAL) {
		l_struc.direction = glm::vec4(direction, 0);
	}
	else if (light_type == SPOTLIGHT) {
		l_struc.position = glm::vec4(position, 0);
		l_struc.direction = glm::vec4(direction, 0);
		l_struc.attenuation = glm::vec4(attenuation, 0);
		l_struc.angle = angle;
	}
	else if (light_type == FLASHLIGHT) {
		l_struc.attenuation = glm::vec4(attenuation, 0);
		l_struc.angle = angle;
	}

	return l_struc;
}
