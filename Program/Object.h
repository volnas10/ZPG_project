#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include <vector>
#include <map>
#include <string>

#include "TextureManager.h"
#include "Util.h"

namespace object {
# pragma pack(push, 1)
	struct Material {
		glm::vec4 has_textures; // diffuse, specular, normal
		glm::vec4 diffuse_color;
		glm::vec4 specular_color;
		glm::vec4 ambient_color;
		float refraction_index;
		float shininess;
		int texture_id;
	};
# pragma pack(pop)
	// Mesh is a part of object that has only 1 material and (not necessarily) texture
	class Mesh {
	private:
		size_t index_count;
		Material material;

		GLuint VBO, VIO;
		GLuint normal_buffer, uv_buffer, tangent_buffer, bitangent_buffer;
		GLuint material_buffer;
	public:
		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
			std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices,
			Material material);
		void bind();
		void bindForShadows();
		void bindUniforms(GLuint material_binding, GLuint diffuse_t, GLuint normal_t, GLuint opacity_t, GLuint has_textures);

		size_t size();
	};

	// Object is a collection of meshes
	class Object {
	private:
		unsigned int next_mesh = 0;
		unsigned int next_trans = 0;
		std::vector<Mesh*> meshes;
	public:
		std::string name;

		Mesh* addMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
			std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices,
			Material material);

		std::vector<Mesh*> getMeshes();
	};
}


#endif OBJECT_H