#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include <vector>
#include <string>

// std140 compatible
struct Material {
	glm::vec4 diffuse_color; // r, g, b | texture_index
	glm::vec4 specular_color;
	glm::vec4 ambient_color;
	glm::vec4 emissive_color;
	glm::vec4 transparent_color; // r, g, b | opacity
	glm::vec4 reflective_color; // r, g, b | reflectivity
	float refraction_index;
	float shininess;
	float shininess_strength;
};

class Object {
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;
	std::vector<Material> materials;
	std::vector<unsigned int> material_indices;

	GLuint VBO, VIO;
	GLuint normal_buffer, uv_buffer;
	GLuint mat_index_buffer, material_ubo;
	std::vector<GLuint> textures;
public:
	std::string name;

	Object(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<unsigned int> indices, std::vector<GLuint> textures, std::vector<Material> materials,
		std::vector<unsigned int> material_indices);

	size_t prepareForDraw();
};

#endif OBJECT_H