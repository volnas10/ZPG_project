#include "Object.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace object {
	Mesh* object::Object::addMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices,
		Material material) {
		
		Mesh* mesh = new Mesh(vertices, normals, uvs, tangents, bitangents, indices, material);
		meshes.push_back(mesh);
		return mesh;
	}

	std::vector<Mesh*> Object::getMeshes() {
		return meshes;
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices,
		Material material) {

		index_count = indices.size();
		this->material = material;

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

			glGenBuffers(1, &tangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

			glGenBuffers(1, &bitangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);
		}
		else {
			uv_buffer = 0;
		}

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

			glEnableVertexAttribArray(3);
			glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);

			glEnableVertexAttribArray(4);
			glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	}

	void Mesh::bindForShadows() {
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		if (uv_buffer > 0) {
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	}

	void Mesh::bindUniforms(GLuint material_binding) {
		glBindBufferBase(GL_UNIFORM_BUFFER, material_binding, material_buffer);
	}

	size_t Mesh::size() {
		return index_count;
	}

	void ObjectGroup::addObjectTransformation(object::Object* object, trans::Transformation* transformation) {
		objects[object].second.push_back(transformation);
	}

	void ObjectGroup::addAllObjectTransformations(object::Object* object, std::pair<trans::Transformation*, std::vector<trans::Transformation*>> transformations) {
		objects[object] = transformations;
	}
}