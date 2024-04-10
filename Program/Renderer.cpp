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

AbstractRenderer::~AbstractRenderer() {
    delete program;
}

/*
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
*/

Renderer::~Renderer() {
    delete program;
}

Renderer::Renderer(Program* program) {
    this->program = program;
    irradiance_ID = program->getUniformLocation("IrradianceSampler");
    prefiltered_map_ID = program->getUniformLocation("PrefilteredMapSampler");
    brdf_ID = program->getUniformLocation("BRDFSampler");
    depth_map_ID = program->getUniformLocation("DepthMaps");
    use_shadows_ID = program->getUniformLocation("ShadowsOn");
}

void Renderer::prepare(int* transformations_idx, GLint depth_map_ID) {
    program->use();
    TextureManager::bind(2);
    glUniform1i(irradiance_ID, 1);
    glUniform1i(prefiltered_map_ID, 2);
    glUniform1i(brdf_ID, 3);
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
    mesh->bindUniforms(1);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei) mesh->size(), GL_UNSIGNED_INT, NULL, (GLsizei) count);

    program->stopUsing();
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

    sampler_ID = program->getUniformLocation("Crosshair");
    aspect_ratio_ID = program->getUniformLocation("AspectRatio");

    TextureManager::addCrosshair("../Resources/crosshair.png");

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

    program->use();
    glUniform1i(sampler_ID, 4);
    program->stopUsing();
}

void CrosshairRenderer::render() {
    program->use();

    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);

    program->stopUsing();
}

void CrosshairRenderer::updateSize(int width, int height) {
    program->use();
    glUniform1f(aspect_ratio_ID, (float)width / height);
    program->stopUsing();
}

EnvMapRenderer::EnvMapRenderer(std::vector<float> sphere) : AbstractRenderer() {
    std::vector<Shader> shaders;
    shaders.push_back(Shader("EnvMapVertexShader.glsl", GL_VERTEX_SHADER));
    shaders.push_back(Shader("EnvMapFragmentShader.glsl", GL_FRAGMENT_SHADER));
    program = new Program(shaders);

    sampler_ID = program->getUniformLocation("EnvMapSampler");

    triangles = sphere.size() / 3;
    glGenBuffers(1, &sphere_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.size() * sizeof(float), &sphere[0], GL_STATIC_DRAW);

    program->use();
    glUniform1i(sampler_ID, 1);
    program->stopUsing();
}

void EnvMapRenderer::render() {
    program->use();
    glDisable(GL_DEPTH_TEST);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    //glUniform1i(texture_samplers[0], GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 0, triangles);

    glEnable(GL_DEPTH_TEST);
    program->stopUsing();
}
