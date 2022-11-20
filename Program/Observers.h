#pragma once
#ifndef OBSERVERS_H
#define OBSERVERS_H

#include <glm/glm.hpp>

class CameraSubscriber {
public:
	virtual void updateCamera(glm::mat4 view_matrix, glm::mat4 projection_matrix) = 0;
};

class CameraPositionSubscriber {
public:
	virtual void updateCameraPosition(glm::vec3 position) = 0;
};

#include "Light.h"

class LightSubscriber {
public:
	virtual void updateLights(std::vector<Light::LightStruct> lights) = 0;
};

#endif
