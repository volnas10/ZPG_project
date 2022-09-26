#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader {
private:
	GLuint shader_ID;
public:
	Shader(std::string shader_path, GLenum type);
	GLuint getID();
};

#endif
