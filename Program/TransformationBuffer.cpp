#include "Util.h"

#include "TransformationBuffer.h"
#include <iostream>

#define MAX_TRANSFORMATIONS 1024

TransformationBuffer::TransformationBuffer() {
	glGenBuffers(1, &buffer_ID);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer_ID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * MAX_TRANSFORMATIONS, NULL, GL_DYNAMIC_DRAW);
	updated = true;
	default_transformation = nullptr;
}

void TransformationBuffer::setDefaultTransformation(trans::Transformation* transformation) {
	default_transformation = transformation;
}

void TransformationBuffer::setTransformations(std::vector<trans::Transformation*> transformations) {
	for (trans::Transformation* t : transformations) {
		addTransformation(t);
	}
}

void TransformationBuffer::addTransformation(trans::Transformation* transformation) {
	if (default_transformation != nullptr) {
		*transformation << *default_transformation;
	}
	transformations.push_back(transformation->getTransformation());
	transformation->addDependency(transformations.size(), this);
	updated = true;
}

void TransformationBuffer::updateTransformation(size_t at, glm::mat4 transformation) {
	transformations[at] = transformation;
	updated = true;
}

void TransformationBuffer::bind(int block_idx) {
	// If some transformation changed, buffer data again
	if (updated) {
		glBindBuffer(GL_UNIFORM_BUFFER, buffer_ID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, transformations.size() * sizeof(glm::mat4), &transformations[0]);
		updated = false;
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, block_idx, buffer_ID);
}

size_t TransformationBuffer::size() {
	return transformations.size();
}

