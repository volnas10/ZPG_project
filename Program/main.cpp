// Include static GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// Include GLFW  
#include <GLFW/glfw3.h>  

// Include the standard C++ headers  
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "Window.h"

static void error_callback(int error, const char* description){ fputs(description, stderr); }

int main(void) {
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1024, 768, "ZPG", NULL, NULL);

	if (!window) {
		std::cout << "Failed to intialize glew" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Get version info
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "Vendor " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	int major, minor, revision;
	glfwGetVersion(&major, &minor, &revision);
	std::cout << "Using GLFW " << major << "." << minor << "." << revision << std::endl;
	
	// Create my window
	Window* my_window = new Window(window);
	my_window->start();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}