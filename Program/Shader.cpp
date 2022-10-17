#include <iostream>
#include <fstream>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>

#include "Shader.h"

Shader::Shader(std::string name, GLenum type) {
	shader_ID = glCreateShader(type);

	std::string shader_code;
	std::ifstream shader_file("./shaders/" + name);
	if (shader_file.is_open()) {
		std::stringstream sstr;
		sstr << shader_file.rdbuf();
		shader_code = sstr.str();
		shader_file.close();
	}
	else {
		std::cout << name << " could not be opened" << std::endl;
	}

	GLint result;

	std::cout << "Compiling shader: " << name << std::endl;
	const char* shader_ptr = shader_code.data();
	glShaderSource(shader_ID, 1, &shader_ptr, NULL);
	glCompileShader(shader_ID);

	glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		std::cout << "Shader compilation failed" << std::endl;

		GLsizei log_length;
		GLchar log[2048];
		glGetShaderInfoLog(shader_ID, 2048, &log_length, log);

		std::cout << log << std::endl;
	}
	else {
		std::cout << "Shader compilation successfull" << std::endl;
	}
}

GLuint Shader::getID() {
	return shader_ID;
}
