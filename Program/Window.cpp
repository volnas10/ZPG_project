#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "Shader.h"

#include "Window.h"

Window::Window(GLFWwindow* window) {
	this->window = window;
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
	glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Move this initialization into renderer class in future
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
    float points[] = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    GLuint VBO;
    glGenBuffers(1, &VBO); // generate the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    GLuint VAO = 0;
    glGenVertexArrays(1, &VAO); //generate the VAO
    glBindVertexArray(VAO); //bind the VAO


    glEnableVertexAttribArray(0); //enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint matrix_ID = program->getUniformLocation("MVP");

    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        program->use();

        handleInput();

        glm::mat4 model_matrix = glm::mat4(1.0f);
        glm::mat4 view_matrix = camera->getView();
        glm::mat4 projection_matrix = camera->getProjection();
        glm::mat4 MVP = projection_matrix * view_matrix * model_matrix;

        glUniformMatrix4fv(matrix_ID, 1, GL_FALSE, &MVP[0][0]);

        glBindVertexArray(VAO);
        // draw triangles
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* actual_window = (Window*)glfwGetWindowUserPointer(window);
    // Do something on key press
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
        dir.z = 1.0f * delta_time;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        dir.z = -1.0f * delta_time;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        dir.x = 1.0f * delta_time;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        dir.x = -1.0f * delta_time;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        dir.y = 1.0f * delta_time;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        dir.y = -1.0f * delta_time;
    }

    camera->move(dir, horizontal_angle, vertical_angle);

    last_time = current_time;
}

