#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <assimp/texture.h>

class Texture
{
public:
	enum Type{DIFFUSE, NORMAL, OPACITY, SPECULAR, SKYBOX};

	void setType(Type t);
	Type getType();
	GLuint getUnit();

	bool load(const char* filename);
	bool loadFromMemory(aiTexture* t);
	bool loadSkybox(std::string path);

	static GLuint reserveUnit();
private:
	static int unit_counter;
	Type texture_type;
	GLuint texture_ID;
	GLuint texture_unit;
};

#endif TEXTURE_H