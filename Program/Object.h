#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include <vector>
#include <string>

class Object {
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;

	GLuint VBO, VIO;
	GLuint normal_buffer, uv_buffer;
	GLuint texture_ID;
public:
	std::string name;

	Object(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs, std::vector<unsigned int> indices, GLuint texture);

	size_t prepareForDraw();
};

#endif OBJECT_H