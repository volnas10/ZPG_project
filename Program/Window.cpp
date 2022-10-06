#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "Shader.h"
#include "Object.h"
#include "ObjectRenderer.h"
#include "Util.h"

#include "Window.h"

#define MOVEMENT_SPEED 2.0f

Window::Window(GLFWwindow* window) {
	this->window = window;

    // Set callbacks
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
    glfwSetScrollCallback(window, scrollCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    
    // Create program vith basic shaders
    std::vector<Shader> shaders;
    shaders.push_back(Shader("./shaders/VertexShader.glsl", GL_VERTEX_SHADER));
    shaders.push_back(Shader("./shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER));
    program = new Program(shaders);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size = glm::vec2(width, height);

    camera = new Camera(glm::vec3(0.0f, 0.0f, -4.0f), 70.0f, 0.0f, 0.0f, (float) width / height);

    last_time = -1;
}

void Window::start() {
    
    // Test cube
    Object test;
    ObjectRenderer renderer(program, camera);

    // Both cubes will have shared height transformation
    float size = 1;
    bool growing = true;
    trans::Transformation cube_trans1;
    auto scale = cube_trans1.scale(1, size, 1);
    renderer.addObject(&test, &cube_trans1);

    // And the second will also get rotated around the first one 
    trans::Transformation cube_trans2;
    float angle = 0;
    cube_trans2.translate(2, 0, 0);
    auto rotation = cube_trans2.rotate(0, angle, 0);

    // Apply first transformation for scale
    cube_trans2 << cube_trans1;
   
    renderer.addObject(&test, &cube_trans2);

    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        handleInput();

        // Call .render() on all renderers (currently just one)
        renderer.render();

        // Rotate cube 2
        angle = fmod(angle + 0.01, 360);

        // Growing and shrinking of both cubes
        if (growing) {
            if (size >= 2) {
                growing = false;
            }
            size += 0.005;
        }
        else {
            if (size <= 1) {
                growing = true;
            }
            size -= 0.005;
        }
        rotation->set(0, angle, 0);
        scale->set(1, size, 1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Window::windowResizeCallback(GLFWwindow* window, int width, int height) {
	Window* actual_window = (Window*)glfwGetWindowUserPointer(window);
	actual_window->windowResized(width, height);
}

void Window::windowResized(int width, int height) {
    window_size = glm::vec2(width, height);
    // Notify all objects that need to know current window size
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Window* actual_window = (Window*)glfwGetWindowUserPointer(window);
    actual_window->scroll(yoffset);
}

void Window::scroll(double yoffset) {
    camera->changeFov(yoffset);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* actual_window = (Window*)glfwGetWindowUserPointer(window);
}

void Window::handleInput() {
    if (last_time < 0) {
        last_time = glfwGetTime();
    }

    double current_time = glfwGetTime();
    float delta_time = float(current_time - last_time);

    double xpos, ypos;

    glfwGetCursorPos(window, &xpos, &ypos);
    glfwSetCursorPos(window, window_size.x / 2, window_size.y / 2);

    float horizontal_angle = 1.0f * delta_time * float(window_size.x / 2 - xpos);
    float vertical_angle = 1.0f * delta_time * float(window_size.y / 2 - ypos);

    glm::vec3 dir(.0f, .0f, .0f);

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        dir.z = MOVEMENT_SPEED * delta_time;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        dir.z = -MOVEMENT_SPEED * delta_time;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        dir.x = MOVEMENT_SPEED * delta_time;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        dir.x = -MOVEMENT_SPEED * delta_time;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        dir.y = MOVEMENT_SPEED * delta_time;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        dir.y = -MOVEMENT_SPEED * delta_time;
    }

    camera->move(dir, horizontal_angle, vertical_angle);

    last_time = current_time;
}

