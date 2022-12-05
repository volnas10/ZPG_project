#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Program.h"
#include "Shader.h"

#include "ShadowMapper.h"

const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096, SHADOW_COUNT = 6;

ShadowMapper::ShadowMapper() {
    // Depth buffer and shadow map shinanigans
    glGenFramebuffers(1, &depth_map_FBO);

    // Make array for all depth maps
    glGenTextures(1, &depth_map);
    texture_unit = Texture::reserveUnit();
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depth_map);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24,
        SHADOW_WIDTH, SHADOW_HEIGHT, SHADOW_COUNT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Failed to create shadow mapper" << std::endl;
        return;
    }

    // Create program
    std::vector<Shader> shaders;
    shaders.push_back(Shader("ShadowVertexShader.glsl", GL_VERTEX_SHADER));
    shaders.push_back(Shader("ShadowFragmentShader.glsl", GL_FRAGMENT_SHADER));
    shadow_program = new Program(shaders);

    pvmatrix_ID = shadow_program->getUniformLocation("PVmatrix");
    texture_ID = shadow_program->getUniformLocation("TextureSampler");
    opacity_ID = shadow_program->getUniformLocation("OpacitySampler");
    has_textures_ID = shadow_program->getUniformLocation("HasTextures");
}

void ShadowMapper::prepare(int* transformation_idx) {
    shadow_program->use();
    *transformation_idx = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glCullFace(GL_FRONT);
}

void ShadowMapper::useLight(Light::LightStruct light, unsigned int index) {
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_map, 0, index);
    glClear(GL_DEPTH_BUFFER_BIT);
    glm::mat4 pvmatrix = light.lightspace_matrix;
    glUniformMatrix4fv(pvmatrix_ID, 1, GL_FALSE, &pvmatrix[0][0]);
}



void ShadowMapper::renderShadows(object::Mesh* mesh, size_t count) {
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)mesh->size(), GL_UNSIGNED_INT, NULL, (GLsizei)count);
}

void ShadowMapper::useTextures(std::vector<Texture*> textures) {
    glm::vec2 has_textures(0.0);
    for (Texture* t : textures) {
        if (t->getType() == Texture::DIFFUSE) {
            glUniform1i(texture_ID, t->getUnit());
            has_textures.x = 1.0;
        }
        else if (t->getType() == Texture::OPACITY) {
            glUniform1i(opacity_ID, t->getUnit());
            has_textures.y = 1.0;
        }
    }
    glUniform2fv(has_textures_ID, 1, &has_textures[0]);
}

GLuint ShadowMapper::getUnit() {
    return texture_unit;
}

GLuint ShadowMapper::getDepthMap() {
    return depth_map;
}
