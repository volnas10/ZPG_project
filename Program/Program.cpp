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

	view_ID = glGetUniformLocation(program_ID, "ViewMatrix");
	projection_view_ID = glGetUniformLocation(program_ID, "ProjectionViewMatrix");
	light_count_ID = glGetUniformLocation(program_ID, "LightCount");

	glGenBuffers(1, &lights_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, lights_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light::LightStruct) * 6, NULL, GL_DYNAMIC_DRAW);
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

void Program::updateCamera(glm::mat4 view_matrix, glm::mat4 projection_matrix) {
	glProgramUniformMatrix4fv(program_ID, view_ID, 1, GL_FALSE, &view_matrix[0][0]);
	glm::mat4 PVmat = projection_matrix * view_matrix;
	glProgramUniformMatrix4fv(program_ID, projection_view_ID, 1, GL_FALSE, &PVmat[0][0]);
}

void Program::updateLights(std::vector<Light::LightStruct> lights) {
	glBindBuffer(GL_UNIFORM_BUFFER, lights_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size() * sizeof(Light::LightStruct), &lights[0]);
	for (int i = 0; i < lights.size(); i++) {
		glBindBufferRange(GL_UNIFORM_BUFFER, 3 + i, lights_buffer, 256 * i, sizeof(Light::LightStruct));
	}

	glProgramUniform1i(program_ID, light_count_ID, lights.size());
}


GLuint Program::getUniformLocation(std::string name) {
	return glGetUniformLocation(program_ID, name.data());
}

GLuint Program::getUniformBlockLocation(std::string name, int pos) {
	GLuint loc = glGetUniformBlockIndex(program_ID, name.c_str());
	glUniformBlockBinding(program_ID, loc, pos);
	return loc;
}
