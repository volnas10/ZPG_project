#pragma once
#ifndef PROGRAM_H
#define PROGRAM_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "Shader.h"

#include <vector>

class Program {
private:
	GLuint program_ID;

	GLuint view_ID, projection_ID;
public:
	Program();
	Program(std::vector<Shader> shaders);
	~Program();

	void use();
	void stopUsing();

	void notify(glm::mat4 view_matrix, glm::mat4 projection_matrix);

	GLuint getUniformLocation(std::string name);
};

#endif
