#pragma once
#ifndef FLOORGENERATOR_H
#define FLOORGENERATOR_H

#include "Texture.h"
#include "Object.h"

#include <vector>

class FloorGenerator {
private:
	int dimension;
	float tile_size;
	std::vector<Texture*> textures;
public:
	FloorGenerator(int dimension, float tile_size, std::vector<Texture*> textures);
	object::Object* generate();
};

#endif

