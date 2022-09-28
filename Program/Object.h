#pragma once
#ifndef OBJECT_H
#define OBJECT_H


class Object {
private:
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	GLuint VBO, VIO;
public:
	Object();

	size_t prepareForDraw();
};

#endif OBJECT_H