#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <assimp/texture.h>

class Texture
{
public:
	enum Type{DIFFUSE, NORMAL, OPACITY, SPECULAR, SKYBOX, HDRI};

	void setType(Type t);
	Type getType();
	GLuint getUnit();

	bool load(const char* filename);
	bool loadFromMemory(aiTexture* t);
	bool loadSkybox(std::string path);
	bool loadHDRI(const char* filename);

	static GLuint reserveUnit();
private:
	static int unit_counter;
	Type texture_type;
	GLuint texture_ID;
	GLuint texture_unit;

	std::string texture_name;
};

class TextureManager
{
public:
private:
	
};
#endif TEXTURE_H