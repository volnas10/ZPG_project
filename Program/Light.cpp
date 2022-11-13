#include "Observers.h"

#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>

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

glm::mat4 Light::getProjectionMatrix() {
	// Orthographic matrix for directional light
	if (light_type == DIRECTIONAL) {
		glm::mat4 projection_matrix = glm::ortho(-10, 10, -10, 10, -20, 20);
		glm::mat4 view_matrix = glm::lookAt(glm::vec3(0, 0, 0), direction, glm::vec3(0, 1, 0));
		return projection_matrix * view_matrix;
	}
	// Perspective projection for spotlight
	else if (light_type == SPOTLIGHT) {
		glm::mat4 projection_matrix = glm::perspective(angle, 1.0f, 0.0f, 100.0f);
		glm::mat4 view_matrix = glm::lookAt(position, position + direction, glm::vec3(0, 1, 0));
		return projection_matrix * view_matrix;
	}
	// Perspective projection in 6 directions for point light
	else if (light_type == POINT) {
		// TODO
		return glm::mat4(1.0);
	}
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
		l_struc.angle_precalculated = cos(angle * 0.00872663889);
	}
	else if (light_type == FLASHLIGHT) {
		l_struc.attenuation = glm::vec4(attenuation, 0);
		l_struc.angle_precalculated = cos(angle * 0.00872663889);
	}

	return l_struc;
}

void LightCollection::addLight(Light light) {
	lights.push_back(light);
}

void LightCollection::subscribe(LightSubscriber* subscriber) {
	subscribers.push_back(subscriber);
}

void LightCollection::notifySubscribers() {
	std::vector<Light::LightStruct> light_structs;
	for (Light l : lights) {
		light_structs.push_back(l.toStruct());
	}
	for (LightSubscriber* sub : subscribers) {
		sub->updateLights(light_structs);
	}
}
