#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <vector>
#include <set>

#include "Util.h"

class LightCollection;

class Light {
public:
	enum Type{POINT, DIRECTIONAL, SPOTLIGHT, FLASHLIGHT};
	struct LightStruct {
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 attenuation; // constant, linear, quadratic, -
		glm::mat4 lightspace_matrix;
		float angle_precalculated; // Little optimization
		unsigned int type;
		int padding[30];
	};
	
	Light();
	void makePoint(glm::vec3 position, glm::vec3 attenuation);
	void makeDirectional(glm::vec3 direction);
	void makeSpotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float angle);
	void makeFlashlight(glm::vec3 attenuation, float angle);
	void setTransformation(trans::Transformation* transformation);

	void setCollection(LightCollection* light_col, size_t pos);
	void update();
	void calculateMatrix();
	glm::mat4 getMatrix();
	Type getType();

	LightStruct toStruct();

private:
	Type light_type;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 attenuation;
	float angle;

	std::pair<LightCollection*, size_t> light_collection;
	trans::Transformation* transformation;
	glm::mat4 lightspace_matrix;
};

#include "Observers.h"

class LightSubscriber;

class LightCollection : public CameraSubscriber {
private:
	std::vector<Light*> lights;
	std::vector<Light::LightStruct> light_structs;
	std::set<int> changed_lights;
	std::vector<LightSubscriber*> subscribers;

	glm::vec3 camera_position;
public:
	~LightCollection();
	void addLight(Light* light);
	void lightChanged(size_t pos);

	void subscribe(LightSubscriber* subscriber);
	void notifySubscribers();

	void updateCamera(glm::mat4 view_matrix, glm::mat4 projection_matrix);

};

#endif