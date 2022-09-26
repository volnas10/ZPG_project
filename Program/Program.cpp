#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <iostream>

#include "Program.h"

Program::Program(std::vector<Shader> shaders) {
	std::cout << "Linking program" << std::endl;

	program_ID = glCreateProgram();
	for (Shader shader : shaders) {
		glAttachShader(program_ID, shader.getID());
	}

	glLinkProgram(program_ID);

	GLint result;
	GLsizei log_length;

	glGetProgramiv(program_ID, GL_LINK_STATUS, &result);
	glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		std::vector<char> error_message(log_length + 1);
		glGetProgramInfoLog(program_ID, log_length, NULL, &error_message[0]);
		printf("%s\n", &error_message[0]);
	}

	for (Shader shader : shaders) {
		glDetachShader(program_ID, shader.getID());
	}
}

Program::~Program() {
	glDeleteProgram(program_ID);
}

void Program::use() {
	glUseProgram(program_ID);
}

void Program::stopUsing() {
	glUseProgram(0);
}

GLuint Program::getUniformLocation(std::string name) {
	return glGetUniformLocation(program_ID, name.data());
}

