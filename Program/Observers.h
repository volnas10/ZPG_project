#pragma once
#ifndef OBSERVERS_H
#define OBSERVERS_H

#include <glm/glm.hpp>
#include <vector>

#include "Light.h"

class CameraSubscriber {
public:
	virtual void updateCamera(glm::mat4 view_matrix, glm::mat4 projection_matrix) = 0;
};

class LightSubscriber {
public:
	virtual void updateLights(std::vector<Light*> lights) = 0;
};
#endif
