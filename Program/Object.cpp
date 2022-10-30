#include "Object.h"

#include <iostream>

namespace object {
	Mesh* object::Object::addMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices, Material material) {
		
		Mesh* mesh = new Mesh(vertices, normals, uvs, tangents, bitangents, indices, material);
		meshes.push_back(std::make_pair(mesh, std::vector<trans::Transformation*>()));
		return mesh;
	}

	void Object::transformMesh(unsigned int index, trans::Transformation* transformation) {
		meshes[index].second.push_back(transformation);
	}

	void Object::transformMesh(unsigned int index, std::vector<trans::Transformation*> transformations) {
		meshes[index].second = transformations;
	}

	bool Object::prepareMesh(size_t* size, GLuint mesh_trans_uniform, GLuint material_uniform) {
		if (next_trans == 0) {
			meshes[next_mesh].first->prepareForDraw();
		}

		glm::mat4 model_matrix(1.0);
		if (meshes[next_mesh].second.size() > 0) {
			model_matrix = meshes[next_mesh].second[next_trans]->getTransformation();
		}
		glUniformMatrix4fv(mesh_trans_uniform, 1, GL_FALSE, &model_matrix[0][0]);

		*size = meshes[next_mesh].first->size();

		if (next_trans + 1 >= meshes[next_mesh].second.size()) {
			next_trans = 0;
			if (next_mesh + 1 >= meshes.size()) {
				next_mesh = 0;
				return false;
			}
			next_mesh++;
		}
		else {
			next_trans++;
		}
		
		return true;
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
		std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices, Material material) {

		//this->vertices = vertices;
		//this->normals = normals;
		//this->uvs = uvs;
		//this->tangents = tangents;
		//this->bitangents = bitangents;
		//this->indices = indices;

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

	void Mesh::prepareForDraw() {
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
		if (material.diffuse_texture > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);
		}
		if (material.normal_map > 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, material.normal_map);
		}
		if (material.opacity_map > 0) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, material.opacity_map);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, material_buffer);
	}

	size_t Mesh::size() {
		//return indices.size();
		return index_count;
	}
}