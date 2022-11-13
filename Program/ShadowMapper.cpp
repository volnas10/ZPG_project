#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <iostream>

#include "Program.h"
#include "Shader.h"

#include "ShadowMapper.h"

ShadowMapper::ShadowMapper() {
    // Depth buffer and shadow map shinanigans
    glGenFramebuffers(1, &depth_map_FBO);

    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    GLuint depth_map;
    glGenTextures(1, &depth_map);
    glBindTexture(GL_TEXTURE_2D, depth_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
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

}

void ShadowMapper::renderShadows() {
    glm::mat4 projection_matrix;
}
