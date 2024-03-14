#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <assimp/texture.h>
#include <vector>

#define MAX_TEXTURE_BLOCKS 32

class TextureManager
{
public:
	enum TextureType { DIFFUSE, NORMAL, SPECULAR};

	static TextureManager& getInstance();
	static void init();

	static void addEnvMap(const char* filename);
	static void addCrosshair(const char* filename);
	static int addMaterial(aiTexture* t, TextureType type); // Returns new material ID
	static int addMaterial(const char* filename, TextureType type); // Returns new material ID
	static int addMaterial(int material_id, aiTexture* t, TextureType type); // Adds to existing material
	static int addMaterial(int material_id, const char* filename, TextureType type); // Adds to existing material
	static void bind(int block_idx);
private:
	GLuint buffer_ID;
	GLuint env_map;
	GLuint irradiance_map;
	GLuint prefiltered_map;
	GLuint crosshair;
	struct TexturePack {
		GLuint64 diffuse;
		GLuint64 specular;
		GLuint64 normal;
		GLuint64 brdf_integration_map;
	};
	std::vector<TexturePack> texture_packs;
	bool updated;
	int id_counter;
};
#endif TEXTURE_H