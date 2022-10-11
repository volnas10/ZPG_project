#include "Object.h"

Object::Object() {
	// Test data
	glm::vec3 test_vertices[] = {
		glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f)
	};
	vertices.insert(vertices.begin(), std::begin(test_vertices), std::end(test_vertices));

	glm::vec3 test_colors[] = {
	glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 1), glm::vec3(0, 0, 1),
	glm::vec3(0, 1, 0), glm::vec3(1, 1, 0), glm::vec3(1, 1, 1), glm::vec3(0, 1, 1)
	};
	colors.insert(colors.begin(), std::begin(test_colors), std::end(test_colors));

	unsigned int test_indices[] = {
		0, 1, 2, 0, 2, 3, // Bottom face
		4, 5, 6, 4, 6, 7, // Top face
		0, 1, 5, 0, 5, 4, // Front face
		2, 3, 7, 2, 7, 6, // Back face
		1, 2, 6, 1, 6, 5, // Right face
		3, 0, 4, 3, 4, 7  // Left face
	};

	indices.insert(indices.begin(), std::begin(test_indices), std::end(test_indices));

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VIO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

size_t Object::prepareForDraw() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIO);

	return indices.size();
}
