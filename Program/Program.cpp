//#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "Program.h"

Program::Program() {
	program_ID = 0;
}

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

	view_ID = glGetUniformLocation(program_ID, "view_matrix");
	projection_ID = glGetUniformLocation(program_ID, "projection_matrix");
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

void Program::notify(glm::mat4 view_matrix, glm::mat4 projection_matrix) {
	glProgramUniformMatrix4fv(program_ID, view_ID, 1, GL_FALSE, &view_matrix[0][0]);
	glProgramUniformMatrix4fv(program_ID, projection_ID, 1, GL_FALSE, &projection_matrix[0][0]);
}



GLuint Program::getUniformLocation(std::string name) {
	return glGetUniformLocation(program_ID, name.data());
}

