#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include <vector>
#include <map>
#include <string>

#include "Util.h"

namespace object {
	struct Material {
		glm::vec4 diffuse_color; // r, g, b, texture_ID
		glm::vec4 specular_color;
		glm::vec4 ambient_color;
		glm::vec4 emissive_color;
		glm::vec4 transparent_color; // r, g, b, opacity
		glm::vec4 reflective_color; // r, g, b, reflectivity
		float refraction_index;
		float shininess;
		float shininess_strength;
	};

	// Mesh is a part of object that has only 1 material and (not necessarily) texture
	class Mesh {
	private:
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<unsigned int> indices;
		Material material;

		GLuint VBO, VIO;
		GLuint normal_buffer, uv_buffer;
	public:
		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
			std::vector<unsigned int> indices, Material material);
		void prepareForDraw();
		size_t size();
	};

	// Object is a collection of meshes
	class Object {
	private:
		unsigned int next_mesh = 0;
		unsigned int next_trans = 0;
		std::vector<std::pair<Mesh*, std::vector<trans::Transformation*>>> meshes;
	public:
		std::string name;

		Mesh* addMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
			std::vector<unsigned int> indices, Material material);
		void transformMesh(unsigned int index, trans::Transformation* transformation);
		void transformMesh(unsigned int index, std::vector<trans::Transformation*> transformations);

		// Each call prepares one mesh, after last meshes has been rendered, returns false
		bool prepareMesh(size_t* size, GLuint mesh_trans_uniform);
	};
}


#endif OBJECT_H