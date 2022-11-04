#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <iostream>
#include <cstdlib>
#include <time.h>

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

	model_matrix_ID = program->getUniformLocation("ModelMatrix");
    mesh_matrix_ID = program->getUniformLocation("MeshMatrix");
    material_ID = program->getUniformBlockLocation("Material", 0);

    texture_samplers.push_back(program->getUniformLocation("TextureSampler"));
    texture_samplers.push_back(program->getUniformLocation("NormalMapSampler"));
    texture_samplers.push_back(program->getUniformLocation("OpacityMapSampler"));
}

void ObjectRenderer::setLights(std::vector<Light> lights) {
    program->use();
    std::vector<Light::LightStruct> light_structs;
    for (Light l : lights) {
        light_structs.push_back(l.toStruct());
    }

    GLuint lights_buffer;
    glGenBuffers(1, &lights_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light::LightStruct) * light_structs.size(), &light_structs[0], GL_DYNAMIC_DRAW);

    for (int i = 0; i < light_structs.size(); i++) {
        glBindBufferRange(GL_UNIFORM_BUFFER, 1 + i, lights_buffer, 256 * i, sizeof(Light::LightStruct));
    }

    GLuint light_count_ID = program->getUniformLocation("LightCount");
    glUniform1i(light_count_ID, light_structs.size());
    program->stopUsing();
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
	//glUniformMatrix3fv(light_ID, 1, GL_FALSE, &light[0][0]);

    // Tell samplers what texture to use
    for (int i = 0; i < texture_samplers.size(); i++) {
        glUniform1i(texture_samplers[i], i);
    }

	for (auto obj : objects) {

        for (auto trans : obj.second) {
            // Use object transformation for all meshes
            glm::mat4 model_matrix = trans->getTransformation();
            glUniformMatrix4fv(model_matrix_ID, 1, GL_FALSE, &model_matrix[0][0]);

            // Draw all meshes
            while (true) {
                size_t index_count;
                bool has_more = obj.first->prepareMesh(&index_count, mesh_matrix_ID, material_ID);
                glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, NULL);
                if (!has_more) break;
            }
        }
	}

	program->stopUsing();
}

SkyboxRenderer::SkyboxRenderer(Program* program, GLuint texture_ID) : AbstractRenderer(program) {
    this->texture_ID = texture_ID;
    texture_samplers.push_back(program->getUniformLocation("Skybox"));

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

    glUniform1i(texture_samplers[0], GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_ID);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glEnable(GL_DEPTH_TEST);
}

FloorRenderer::FloorRenderer(Program* program, float size, int dimension, GLuint texture) : AbstractRenderer(program) {
    this->program = program;
    texture_ID = texture;

    texture_samplers.push_back(program->getUniformLocation("TextureSampler"));

    float half_size = size / 2;

    float vertices[] = {
        -half_size , 0, -half_size,
        half_size, 0, -half_size,
        -half_size, 0, half_size,

        half_size , 0, -half_size,
        half_size, 0, half_size,
        -half_size, 0, half_size
    };

    srand(time(NULL));
    std::vector<glm::vec2> offsets;
    std::vector<float> rotations; // Doesn't work with integers for some reason
    float begin = (size * dimension) / 2 - half_size;
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            offsets.push_back(glm::vec2(i * size - begin, j * size - begin));
            // Rotate tiles randomly
            rotations.push_back(rand() % 4);
        }
    }

     tile_count = offsets.size();

    glGenBuffers(1, &plane_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, plane_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &offset_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, offset_buffer);
    glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(glm::vec2), &offsets[0], GL_STATIC_DRAW);

    glGenBuffers(1, &rotation_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, rotation_buffer);
    glBufferData(GL_ARRAY_BUFFER, rotations.size() * sizeof(float), &rotations[0], GL_STATIC_DRAW);
}


void FloorRenderer::setLights(std::vector<Light> lights) {
    this->lights = lights;
}

void FloorRenderer::render() {
    program->use();

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, plane_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, offset_buffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, rotation_buffer);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribDivisor(2, 1);

    glUniform1i(texture_samplers[0], 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_ID);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, tile_count);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);

    program->stopUsing();
}
