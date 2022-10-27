#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <iostream>

#include "Camera.h"

#include "Renderer.h"

GLuint AbstractRenderer::VAO = 0;

AbstractRenderer::AbstractRenderer(Program* program) {
	this->program = program;
	if (VAO == 0) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
	}
}

ObjectRenderer::ObjectRenderer(Program* program) : AbstractRenderer(program) {

	model_matrix_ID = program->getUniformLocation("model_matrix");
	light_ID = program->getUniformLocation("light_matrix");
	texture_sampler_ID = program->getUniformLocation("texture_sampler");
    mesh_matrix_ID = program->getUniformLocation("mesh_matrix");
    material_ID = program->getUniformBlockLocation("material", 0);

}

void ObjectRenderer::setLight(glm::mat3 light) {
	this->light = light;
}

void ObjectRenderer::addObject(object::Object* obj, trans::Transformation* trans) {
	objects[obj].push_back(trans);
}

void ObjectRenderer::addObject(object::Object* obj, std::vector<trans::Transformation*> transformations) {
	objects[obj] = transformations;
}

void ObjectRenderer::render() {
	program->use();

	// Set light
	glUniformMatrix3fv(light_ID, 1, GL_FALSE, &light[0][0]);

	for (auto obj : objects) {

        for (auto trans : obj.second) {
            // Use object transformation for all meshes
            glm::mat4 model_matrix = trans->getTransformation();
            glUniformMatrix4fv(model_matrix_ID, 1, GL_FALSE, &model_matrix[0][0]);

            // Draw all meshes
            while (true) {
                size_t index_count;
                glActiveTexture(GL_TEXTURE0);
                bool has_more = obj.first->prepareMesh(&index_count, mesh_matrix_ID, material_ID);
                glUniform1i(texture_sampler_ID, GL_TEXTURE0);
                glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, NULL);
                if (!has_more) break;
            }
        }
	}

	program->stopUsing();
}

SkyboxRenderer::SkyboxRenderer(Program* program, GLuint texture_ID) : AbstractRenderer(program) {
    this->texture_ID = texture_ID;
    texture_sampler_ID = program->getUniformLocation("skybox");

    float vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

	glGenBuffers(1, &cube_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void SkyboxRenderer::render() {
    program->use();
    glDisable(GL_DEPTH_TEST);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_ID);

    glUniform1i(texture_sampler_ID, GL_TEXTURE0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glEnable(GL_DEPTH_TEST);
}
