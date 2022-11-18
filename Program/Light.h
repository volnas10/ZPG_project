#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <vector>

class Light {
public:
	enum Type{POINT, DIRECTIONAL, SPOTLIGHT, FLASHLIGHT};
	struct LightStruct {
		glm::vec4 color; // vec3 color + float power
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 attenuation; // constant, linear, quadratic
		float angle_precalculated; // Little optimization
		unsigned int type;
		int padding[46];
	};
	
	Light(glm::vec3 color);
	void makePoint(glm::vec3 position, glm::vec3 attenuation);
	void makeDirectional(glm::vec3 direction);
	void makeSpotlight(glm::vec3 position, glm::vec3 direction, glm::vec3 attenuation, float angle);
	void makeFlashlight(glm::vec3 attenuation, float angle);

	glm::mat4 getMatrices();

	LightStruct toStruct();

private:
	Type light_type;
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 attenuation;
	float angle;
};

class LightSubscriber;

class LightCollection {
private:
	std::vector<Light*> lights;
	std::vector<LightSubscriber*> subscribers;
public:
	~LightCollection();
	void addLight(Light* light);

	void subscribe(LightSubscriber* subscriber);
	void notifySubscribers();

};

#endif