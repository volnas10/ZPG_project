#pragma once
#ifndef PROGRAM_H
#define PROGRAM_H

#include "Shader.h"

#include <vector>

class Program {
private:
	GLuint program_ID;
public:
	Program();
	Program(std::vector<Shader> shaders);
	~Program();
	void use();
	void stopUsing();

	GLuint getUniformLocation(std::string name);
};

#endif
