#pragma once
#ifndef TRANSFORMATIONBUFFER_H
#define TRANSFORMATIONBUFFER_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <vector>
#include <glm/glm.hpp>

namespace trans {
	class Transformation;
}

// Class that takes care of all transformations for one mesh efficiently
class TransformationBuffer {
private:
	std::vector<glm::mat4> transformations;

	GLuint buffer_ID;

	bool updated;
public:
	TransformationBuffer();
	void setTransformations(std::vector<trans::Transformation*> transformations);
	void addTransformation(trans::Transformation* transformation);
	void updateTransformation(size_t at, glm::mat4 transformation);

	void bind(int block_idx);
	size_t size();

};

#endif