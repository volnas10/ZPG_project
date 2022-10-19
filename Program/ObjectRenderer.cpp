#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Camera.h"

#include "ObjectRenderer.h"

GLuint ObjectRenderer::VAO = 0;

ObjectRenderer::ObjectRenderer(Program* program) {
	this->program = program;

	// Create shared VAO for all renderers if not initialized
	if (VAO == 0) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
	}

	model_matrix_ID = program->getUniformLocation("model_matrix");
	light_ID = program->getUniformLocation("light_matrix");
	texture_sampler_ID = program->getUniformLocation("texture_sampler");
}

void ObjectRenderer::setLight(glm::mat3 light) {
	this->light = light;
}

void ObjectRenderer::setSkybox(Skybox* skybox) {
	this->skybox = skybox;
}

void ObjectRenderer::addObject(Object* obj, trans::Transformation* trans) {
	objects[obj].push_back(trans);
}

void ObjectRenderer::addObject(Object* obj, std::vector<trans::Transformation*> transformations) {
	objects[obj] = transformations;
}

void ObjectRenderer::render() {
	program->use();

	// Set light
	glUniformMatrix3fv(light_ID, 1, GL_FALSE, &light[0][0]);

	// Use texture unit 0
	glUniform1i(texture_sampler_ID, 0);



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
