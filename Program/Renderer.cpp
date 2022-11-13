#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <iostream>
#include <cstdlib>
#include <time.h>

#include "Camera.h"

#include "Renderer.h"

AbstractRenderer::AbstractRenderer(Program* program) {
	this->program = program;
}

SkyboxRenderer::SkyboxRenderer(Program* program, Texture* texture) : AbstractRenderer(program) {
    this->texture = texture;
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

    glUniform1i(texture_samplers[0], texture->getUnit());
}

void SkyboxRenderer::render() {
    program->use();
    glDisable(GL_DEPTH_TEST);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glEnable(GL_DEPTH_TEST);
}

FloorRenderer::FloorRenderer(Program* program, float size, int dimension, Texture* texture) : AbstractRenderer(program) {
    this->program = program;
    this->texture = texture;

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

    srand( (unsigned int) time(NULL));
    std::vector<glm::vec2> offsets;
    std::vector<float> rotations; // Doesn't work with integers for some reason
    float begin = (size * dimension) / 2 - half_size;
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            offsets.push_back(glm::vec2(i * size - begin, j * size - begin));
            // Rotate tiles randomly
            rotations.push_back((float) (rand() % 4));
        }
    }

    tile_count = (int) offsets.size();

    glGenBuffers(1, &plane_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, plane_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

     glGenBuffers(1, &offset_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, offset_buffer);
    glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(glm::vec2), &offsets[0], GL_STATIC_DRAW);

    glGenBuffers(1, &rotation_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, rotation_buffer);
    glBufferData(GL_ARRAY_BUFFER, rotations.size() * sizeof(float), &rotations[0], GL_STATIC_DRAW);

    program->use();
    glUniform1i(texture_samplers[0], texture->getUnit());
    program->stopUsing();
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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, tile_count);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);

    program->stopUsing();
}

Renderer::Renderer(Program* program) {
    this->program = program;

    has_textures_ID = program->getUniformLocation("HasTextures");
    diffuse_ID = program->getUniformLocation("DiffuseTextureSampler");
    normal_ID = program->getUniformLocation("NormalTextureSampler");
    opacity_ID = program->getUniformLocation("OpacityTextureSampler");
}

void Renderer::prepare(int* transformations_idx) {
    program->use();
    // Block index of transformations
    *transformations_idx = 0;
}

void Renderer::render(object::Mesh* mesh, size_t count) {
    mesh->bindUniforms(1, diffuse_ID, normal_ID, opacity_ID, has_textures_ID);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei) mesh->size(), GL_UNSIGNED_INT, NULL, (GLsizei) count);

    program->stopUsing();
}

RenderingGroup::RenderingGroup(Program* program) {
    renderer = new Renderer(program);;
}

void RenderingGroup::addObjectTransformation(object::Object* object, trans::Transformation* transformation) {
    objects[object].push_back(transformation);
}

void RenderingGroup::addAllObjectTransformations(object::Object* object, std::vector<trans::Transformation*> transformations) {
    objects[object] = transformations;
}

std::vector<trans::Transformation*> RenderingGroup::getTransformations(object::Object* object) {
    if (objects.find(object) == objects.end()) {
        return std::vector<trans::Transformation*>();
    }
    return objects[object];
}

Renderer* RenderingGroup::getRenderer() {
    return renderer;
}
