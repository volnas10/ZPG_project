#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "Object.h"

struct Transformation {
	int x, y, z;
	int x_rotation, y_rotation, z_rotation;
	float scale;

	glm::mat4 getModelMatrix();
};

struct ObjectTransformation {
	Object* object;
	std::vector<Transformation> transformations;
};

class Scene {
private:

public:

};

#endif SCENE_H