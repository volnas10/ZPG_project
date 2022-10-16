#include "Object.h"

Object::Object(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> uvs, std::vector<unsigned int> indices, GLuint texture) {
	this->vertices = vertices;
	this->normals = normals;
	this->uvs = uvs;
	this->indices = indices;
	texture_ID = texture;

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

	if (texture_ID != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_ID);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);

	return indices.size();
}
