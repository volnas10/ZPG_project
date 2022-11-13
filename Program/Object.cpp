#include "Object.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace object {
	Mesh* object::Object::addMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices,
		Material material, std::vector<Texture*> textures) {
		
		Mesh* mesh = new Mesh(vertices, normals, uvs, tangents, bitangents, indices, material, textures);
		meshes.push_back(mesh);
		return mesh;
	}

	std::vector<Mesh*> Object::getMeshes() {
		return meshes;
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices,
		Material material, std::vector<Texture*> textures) {

		index_count = indices.size();
		this->material = material;
		this->textures = textures;

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

		if (uvs.size() > 0) {
			glGenBuffers(1, &uv_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		}
		else {
			uv_buffer = 0;
		}

		glGenBuffers(1, &tangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &bitangent_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
		glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &VIO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &material_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, material_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(material), &material, GL_DYNAMIC_DRAW);
	}

	void Mesh::bind() {
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		if (uv_buffer > 0) {
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		}

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		// Add textrue manager later

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	}

	void Mesh::bindUniforms(GLuint material_binding, GLuint diffuse_t, GLuint normal_t, GLuint opacity_t, GLuint has_textures) {
		glBindBufferBase(GL_UNIFORM_BUFFER, material_binding, material_buffer);
		glm::vec3 enabled(0.0);
		for (Texture* t : textures) {
			if (t->getType() == Texture::DIFFUSE) {
				glUniform1i(diffuse_t, t->getUnit());
				enabled.x = 1;
			}
			else if (t->getType() == Texture::NORMAL) {
				glUniform1i(normal_t, t->getUnit());
				enabled.y = 1;
			}
			else if (t->getType() == Texture::OPACITY) {
				glUniform1i(opacity_t, t->getUnit());
				enabled.z = 1;
			}
		}
		glUniform3fv(has_textures, 1, glm::value_ptr(enabled));
	}

	size_t Mesh::size() {
		return index_count;
	}
}