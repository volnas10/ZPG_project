#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>

Light::Light(glm::vec3 color) {
	this->color = color;
	transformation = nullptr;
}

void Light::makePoint(glm::vec3 position, glm::vec3 attenuation) {
	this->position = position;
	this->attenuation = attenuation;
	this->light_type = POINT;
	calculateMatrix();
}

void Light::makeDirectional(glm::vec3 direction) {
	this->direction = direction;
	this->light_type = DIRECTIONAL;
	calculateMatrix();
}

void Light::makeSpotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float angle) {
	this->position = position;
	this->direction = direction;
	this->attenuation = attenuation;
	this->angle = angle;
	this->light_type = SPOTLIGHT;
	calculateMatrix();
}

void Light::makeFlashlight(glm::vec3 attenuation, float angle) {
	this->attenuation = attenuation;
	this->angle = angle;
	this->light_type = FLASHLIGHT;
	calculateMatrix();
}

void Light::setTransformation(trans::Transformation* transformation) {
	this->transformation = transformation;
	transformation->addDependency(this);
}

glm::mat4 Light::getMatrix() {
	return lightspace_matrix;
}

Light::LightStruct Light::toStruct() {
	LightStruct l_struc;
	l_struc.type = light_type;
	l_struc.color = glm::vec4(color, 0);

	glm::vec3 dir;
	glm::vec3 pos;
	if (transformation != nullptr) {
		dir = glm::vec3(transformation->getTransformation() * glm::vec4(direction, 1));
		pos = glm::vec3(transformation->getTransformation() * glm::vec4(position, 1));
	}

	if (light_type == POINT) {
		l_struc.position = glm::vec4(pos, 0);
		l_struc.attenuation = glm::vec4(attenuation, 0);
	}
	else if (light_type == DIRECTIONAL) {
		l_struc.direction = glm::vec4(dir, 0);
	}
	else if (light_type == SPOTLIGHT) {
		l_struc.position = glm::vec4(pos, 0);
		l_struc.direction = glm::vec4(dir, 0);
		l_struc.attenuation = glm::vec4(attenuation, 0);
		l_struc.angle_precalculated = cos(angle * 0.00872663889);
	}
	else if (light_type == FLASHLIGHT) {
		l_struc.attenuation = glm::vec4(attenuation, 0);
		l_struc.angle_precalculated = cos(angle * 0.00872663889);
	}

	return l_struc;
}

Light::Type Light::getType() {
	return light_type;
}

void Light::setCollection(LightCollection* light_col, size_t pos) {
	light_collection = std::make_pair(light_col, pos);
}

void Light::update() {
	calculateMatrix();
	light_collection.first->lightChanged(light_collection.second);
}

void Light::calculateMatrix() {
	glm::vec3 dir;
	glm::vec3 pos;
	if (transformation != nullptr) {
		dir = glm::vec3(transformation->getTransformation() * glm::vec4(direction, 1));
		pos = glm::vec3(transformation->getTransformation() * glm::vec4(position, 1));
	}
	// Orthographic matrix for directional light
	if (light_type == DIRECTIONAL) {
		glm::mat4 projection_matrix = glm::ortho(-20.0, 20.0, -20.0, 20.0, -20.0, 20.0);
		glm::mat4 view_matrix = glm::lookAt(glm::vec3(0, 0, 0), dir, glm::vec3(0, 1, 0));
		lightspace_matrix = projection_matrix * view_matrix;
	}
	// Perspective projection for spotlight
	else if (light_type == SPOTLIGHT) {
		glm::mat4 projection_matrix = glm::perspective(angle, 1.0f, 0.0f, 100.0f);
		glm::mat4 view_matrix = glm::lookAt(pos, pos + dir, glm::vec3(0, 1, 0));
		lightspace_matrix = projection_matrix * view_matrix;
	}
	// Perspective projection in 6 directions for point light
	else if (light_type == POINT) {
		// TODO
	}
}

LightCollection::~LightCollection() {
	for (Light* l : lights) {
		delete l;
	}
}

void LightCollection::addLight(Light* light) {
	lights.push_back(light);
	light->setCollection(this, lights.size() - 1);
	light_structs.push_back(light->toStruct());
	changed_lights.insert(lights.size() - 1);
	notifySubscribers();
}

void LightCollection::lightChanged(size_t pos) {
	light_structs[pos] = lights[pos]->toStruct();
	if (lights[pos]->getType() == Light::DIRECTIONAL) {
		light_structs[pos].lightspace_matrix = glm::translate(lights[pos]->getMatrix(), light_offset);
	}
	notifySubscribers();
}

void LightCollection::subscribe(LightSubscriber* subscriber) {
	subscribers.push_back(subscriber);
	std::set<int> indices;
	for (int i = 0; i < lights.size(); i++) {
		indices.insert(i);
	}
	subscriber->updateLights(light_structs);
}

void LightCollection::notifySubscribers() {
	for (LightSubscriber* sub : subscribers) {
		sub->updateLights(light_structs);
	}
}

void LightCollection::updateCameraPosition(glm::vec3 position) {
	light_offset = -position;
	// Directional light will be following camera so only the part of scene around them gets shadows
	for (int i = 0; i < lights.size(); i++) {
		if (lights[i]->getType() == Light::DIRECTIONAL) {
			light_structs[i].lightspace_matrix = glm::translate(lights[i]->getMatrix(), light_offset);
		}
	}
	notifySubscribers();
}
