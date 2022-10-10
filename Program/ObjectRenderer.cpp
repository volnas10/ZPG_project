#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "ObjectRenderer.h"

GLuint ObjectRenderer::VAO = 0;

ObjectRenderer::ObjectRenderer(Program* program, Camera* camera) {
	this->program = program;

	// Create shared VAO for all renderers if not initialized
	if (VAO == 0) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
	}

	model_matrix_ID = program->getUniformLocation("ModelMatrix");
}

void ObjectRenderer::addObject(Object* obj, trans::Transformation* trans) {
	objects[obj].push_back(trans);
}

void ObjectRenderer::render() {
	program->use();

	for (auto obj : objects) {
		// Prepare object for rendering
		size_t index_count = obj.first->prepareForDraw();

		for (auto trans : obj.second) {
			// Draw that object with every transformation
			glm::mat4 model_matrix = trans->getTransformation();

			glUniformMatrix4fv(model_matrix_ID, 1, GL_FALSE, &model_matrix[0][0]);

			// Draw triangles
			glDrawElements(GL_TRIANGLES, (GLsizei) index_count, GL_UNSIGNED_INT, NULL);
		}
	}

	program->stopUsing();
}
