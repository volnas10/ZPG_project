#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "Window.h"
#include "RenderingScheduler.h"

#define MOVEMENT_SPEED 2.0f

Window::Window(GLFWwindow* window) {
	this->window = window;
    this->cursor_locked = true;
    this->frame_count = 0;
    // Set callbacks
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
    glfwSetScrollCallback(window, scrollCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size = glm::vec2(width, height);

    last_time = -1;

    scene = new Scene("shadow_volume_test");
    scene->load();
    
    camera = scene->getCamera();
    
    for (WindowSizeSubscriber* sub : scene->getWindowSubscribers()) {
        subscribe(sub);
	}

    TextureManager tm;
    tm.init();
}

Window::~Window() {
    delete scene;
}

void Window::start() {

    RenderingScheduler rendering_scheduler(scene->getObjectGroup(), scene->getRenderers());
    scene->getLights()->subscribe(&rendering_scheduler);
    rendering_scheduler.setShadowType(scene->getShadowType());

    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {

        scene->moveObjects(glfwGetTime() - last_time);
        handleInput();

        // Render everything
        rendering_scheduler.render(window_size.x, window_size.y);
        frame_count++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    std::cout << "Average framerate: " << frame_count / total_time << " FPS" << std::endl;
}

void Window::subscribe(WindowSizeSubscriber* subscriber) {
    subscribers.push_back(subscriber);
    subscriber->updateSize(window_size.x, window_size.y);
}

void Window::windowResizeCallback(GLFWwindow* window, int width, int height) {
	Window* actual_window = (Window*)glfwGetWindowUserPointer(window);
	actual_window->windowResized(width, height);
}

void Window::windowResized(int width, int height) {
    window_size = glm::vec2(width, height);
    for (WindowSizeSubscriber* sub : subscribers) {
        sub->updateSize(width, height);
    }
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
    actual_window->keyPressed(key, scancode, action, mods);
}

void Window::keyPressed(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // TAB releases user's mouse
        if (key == GLFW_KEY_TAB) {
            if (cursor_locked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                glfwSetCursorPos(window, window_size.x / 2, window_size.y / 2);
            }
            cursor_locked = !cursor_locked;
        }
    }
}

void Window::handleInput() {
    if (last_time < 0) {
        last_time = glfwGetTime();
    }

    double current_time = glfwGetTime();
    float delta_time = float(current_time - last_time);
    total_time += delta_time;

    float horizontal_angle = 0;
    float vertical_angle = 0;
    if (cursor_locked) {
        double xpos, ypos;

        glfwGetCursorPos(window, &xpos, &ypos);
        glfwSetCursorPos(window, window_size.x / 2, window_size.y / 2);

        horizontal_angle = 1.0f * delta_time * float((int) window_size.x / 2 - xpos);
        vertical_angle = 1.0f * delta_time * float((int) window_size.y / 2 - ypos);
    }

    glm::vec3 dir(.0f, .0f, .0f);

    bool moved = false;
    float movement_speed = MOVEMENT_SPEED;
    // Boost movement sapeed
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        movement_speed *= 4;
    }



    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        dir.z = movement_speed * delta_time;
        moved = true;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        dir.z = -movement_speed * delta_time;
        moved = true;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        dir.x = movement_speed * delta_time;
        moved = true;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        dir.x = -movement_speed * delta_time;
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        dir.y = movement_speed * delta_time;
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        dir.y = -movement_speed * delta_time;
        moved = true;
    }

    last_time = current_time;
    // Stop when there's no movement
    if (!moved && horizontal_angle == 0 && vertical_angle == 0) return;

    camera->move(dir, horizontal_angle, vertical_angle);
}

