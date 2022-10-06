#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Object.h"

#include "ObjectRenderer.h"

GLuint ObjectRenderer::VAO = 0;

ObjectRenderer::ObjectRenderer(Program* program, Camera* camera) {
	this->program = program;
	this->camera = camera;

	// Create shared VAO for all renderers if not initialized
	if (VAO == 0) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
	}

	matrix_ID = program->getUniformLocation("MVP");
}

void ObjectRenderer::addObject(Object* obj, trans::Transformation* trans) {
	objects[obj].push_back(trans);
}

void ObjectRenderer::render() {
	program->use();

	glm::mat4 view_matrix = camera->getView();
	glm::mat4 projection_matrix = camera->getProjection();

	for (auto obj : objects) {
		// Prepare object for rendering
		size_t index_count = obj.first->prepareForDraw();

		for (auto trans : obj.second) {
			// Draw that object with every transformation
			glm::mat4 model_matrix = trans->getTransformation();
			glm::mat4 MVP = projection_matrix * view_matrix * model_matrix;

			glUniformMatrix4fv(matrix_ID, 1, GL_FALSE, &MVP[0][0]);

			// Draw triangles
			glDrawElements(GL_TRIANGLES, (GLsizei) index_count, GL_UNSIGNED_INT, NULL);
		}
	}

	program->stopUsing();
}
