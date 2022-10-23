#include "Object.h"

Object::Object(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs,
	std::vector<unsigned int> indices, std::vector<GLuint> textures, std::vector<Material> materials,
	std::vector<unsigned int> material_indices) {
	this->vertices = vertices;
	this->normals = normals;
	this->uvs = uvs;
	this->indices = indices;
	this->textures = textures;
	this->materials = materials;
	this->material_indices = material_indices;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VIO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &mat_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mat_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, material_indices.size() * sizeof(unsigned int), &material_indices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &material_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, material_ubo);
	glBufferData(GL_UNIFORM_BUFFER, materials.size() * sizeof(Material), NULL, GL_STATIC_DRAW);
}

size_t Object::prepareForDraw() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, mat_index_buffer);
	glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, NULL);

	// Bind all textures
	if (textures.size() > 0) {
		glBindTextures(GL_TEXTURE0, textures.size(), &textures[0]);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, material_ubo, 0, materials.size());
	glBindBuffer(GL_UNIFORM_BUFFER, material_ubo);

	return indices.size();
}
