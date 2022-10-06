#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include <vector>

class Object {
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors; // Temporary to show transformations
	std::vector<unsigned int> indices;

	GLuint VBO, VIO, color_buffer;
public:
	Object();

	size_t prepareForDraw();
};

#endif OBJECT_H