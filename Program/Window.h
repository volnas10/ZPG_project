#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <glm/glm.hpp>

#include "ObjectRenderer.h"
#include "Program.h"
#include "Camera.h"

class UI;

class Window {
private:
	glm::vec2 window_size;
	GLFWwindow* window;

	std::vector<ObjectRenderer*> renderers;
	std::vector<Program*> programs;

	Camera* camera;

	double last_time;

	static void windowResizeCallback(GLFWwindow* window, int width, int height);
	void windowResized(int width, int height);

	// Handle scroll
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void scroll(double yoffset);

	// Handles single key presses
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	// Handles key presses every frame
	void handleInput();

public:
	Window(GLFWwindow* window);
	~Window();

	void start();
};

#endif