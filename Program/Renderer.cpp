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

    program->use();
    glUniform1i(texture_samplers[0], texture->getUnit());
    program->stopUsing();
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


Renderer::Renderer(Program* program) {
    this->program = program;

    has_textures_ID = program->getUniformLocation("HasTextures");
    diffuse_ID = program->getUniformLocation("DiffuseTextureSampler");
    normal_ID = program->getUniformLocation("NormalTextureSampler");
    opacity_ID = program->getUniformLocation("OpacityTextureSampler");
    depth_map_ID = program->getUniformLocation("DepthMaps");
    use_shadows_ID = program->getUniformLocation("ShadowsOn");
}

void Renderer::prepare(int* transformations_idx, GLint depth_map_ID) {
    program->use();
    if (depth_map_ID >= 0) {
        glUniform1i(this->depth_map_ID, depth_map_ID);
        glUniform1i(this->use_shadows_ID, GL_TRUE);
    }
    else {
        glUniform1i(this->use_shadows_ID, GL_FALSE);
    }
    // Block index of transformations
    *transformations_idx = 0;
}

void Renderer::render(object::Mesh* mesh, size_t count) {
    mesh->bindUniforms(1, diffuse_ID, normal_ID, opacity_ID, has_textures_ID);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei) mesh->size(), GL_UNSIGNED_INT, NULL, (GLsizei) count);

    program->stopUsing();
}

int RenderingGroup::id_counter = 0;

RenderingGroup::RenderingGroup(Program* program) {
    renderer = new Renderer(program);
    id = id_counter++;
}

void RenderingGroup::addObjectTransformation(object::Object* object, trans::Transformation* transformation) {
    objects[object].second.push_back(transformation);
}

void RenderingGroup::addAllObjectTransformations(object::Object* object, std::pair<trans::Transformation*, std::vector<trans::Transformation*>> transformations) {
    objects[object] = transformations;
}

std::vector<trans::Transformation*> RenderingGroup::getTransformations(object::Object* object) {
    if (objects.find(object) == objects.end()) {
        return std::vector<trans::Transformation*>();
    }
    return objects[object].second;
}

trans::Transformation* RenderingGroup::getDefaultTransformation(object::Object* object) {
    if (objects.find(object) == objects.end()) {
        return nullptr;
    }
    return objects[object].first;
}

Renderer* RenderingGroup::getRenderer() {
    return renderer;
}

DepthMapRenderer::DepthMapRenderer() {
    std::vector<Shader> shaders;
    shaders.push_back(Shader("DepthMapVertexShader.glsl", GL_VERTEX_SHADER));
    shaders.push_back(Shader("DepthMapFragmentShader.glsl", GL_FRAGMENT_SHADER));
    program = new Program(shaders);

    sampler_ID = program->getUniformLocation("DepthMap");

    float quad_vertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
}

void DepthMapRenderer::render(GLuint depth_map_ID) {
    program->use();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glUniform1i(sampler_ID, depth_map_ID);


    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    program->stopUsing();
}

CrosshairRenderer::CrosshairRenderer() : AbstractRenderer() {
    std::vector<Shader> shaders;
    shaders.push_back(Shader("CrosshairVertexShader.glsl", GL_VERTEX_SHADER));
    shaders.push_back(Shader("CrosshairFragmentShader.glsl", GL_FRAGMENT_SHADER));
    program = new Program(shaders);
    float quad_vertices[] = {
        // positions        // texture Coords
        -0.02f,  0.02f, 0.0f, 0.0f, 1.0f,
        -0.02f, -0.02f, 0.0f, 0.0f, 0.0f,
         0.02f,  0.02f, 0.0f, 1.0f, 1.0f,
         0.02f, -0.02f, 0.0f, 1.0f, 0.0f,
    };

    texture_samplers.push_back(program->getUniformLocation("Crosshair"));
    aspect_ratio_ID = program->getUniformLocation("AspectRatio");

    crosshair_texture = new Texture();
    crosshair_texture->load("../Resources/crosshair.png");
    crosshair_texture->setType(Texture::DIFFUSE);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
}

void CrosshairRenderer::render() {
    program->use();

    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glUniform1i(texture_samplers[0], crosshair_texture->getUnit());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);

    program->stopUsing();
}

void CrosshairRenderer::updateSize(int width, int height) {
    program->use();
    glUniform1f(aspect_ratio_ID, (float)width / height);
    program->stopUsing();
}
