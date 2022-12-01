#include <FreeImage.h>
#include <string>
#include <iostream>

#include "Texture.h"

int Texture::unit_counter = 0;

void Texture::setType(Type t) {
	texture_type = t;
}

Texture::Type Texture::getType() {
	return texture_type;
}

GLuint Texture::getUnit() {
	return texture_unit;
}

bool Texture::load(const char* filename) {
	texture_name = std::string(filename);
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* dib(0);
	unsigned char* data(0);
	unsigned int width(0), height(0);
	std::cout << "Loading texture from file: " << filename << std::endl;

	format = FreeImage_GetFileType(filename, 0);

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(filename);
	}
	if (format == FIF_UNKNOWN) {
		std::cout << "Uknown texture format: " << filename << std::endl;
		return false;
	}

	if (FreeImage_FIFSupportsReading(format)) {
		dib = FreeImage_Load(format, filename);
	}
	if (!dib) {
		std::cout << "Could not load texture: " << filename << std::endl;
		return false;
	}

	FREE_IMAGE_COLOR_TYPE fi_color_type = FreeImage_GetColorType(dib);
	GLenum gl_color_type;
	if (fi_color_type == FIC_RGB) {
		gl_color_type = GL_BGR;
	}
	else if (fi_color_type == FIC_RGBALPHA) {
		gl_color_type = GL_BGRA;
	}
	else if (fi_color_type == FIC_MINISBLACK) {
		dib = FreeImage_ConvertTo24Bits(dib);
		gl_color_type = GL_RGB;
	}
	else {
		std::cout << "Unsupported color type: " << filename << std::endl;
		FreeImage_Unload(dib);
		return false;
	}

	data = FreeImage_GetBits(dib);

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	glGenTextures(1, &texture_ID);
	// Bind to texture unit and increment counter
	texture_unit = unit_counter++;
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_color_type, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	FreeImage_Unload(dib);


	return true;
}

bool Texture::loadFromMemory(aiTexture* t) {
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* dib(0);
	unsigned char* data(0);
	unsigned int width(0), height(0);

	std::cout << "Loading texture from memory: " << t->mFilename.C_Str() << std::endl;

	std::string format_name(t->achFormatHint);
	// FreeImage doesn't recognize .jpg
	if (format_name == "jpg") {
		format_name = "jpeg";
	}

	format = FreeImage_GetFIFFromFormat(format_name.c_str());
	FIMEMORY* memory = FreeImage_OpenMemory(reinterpret_cast<BYTE*>(t->pcData), t->mWidth);
	dib = FreeImage_LoadFromMemory(format, memory);
	FreeImage_CloseMemory(memory);

	if (!dib) {
		std::cout << "Could not load texture: " << t->mFilename.C_Str() << std::endl;
		return false;
	}

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	data = FreeImage_GetBits(dib);

	FREE_IMAGE_COLOR_TYPE fi_color_type = FreeImage_GetColorType(dib);
	GLenum gl_color_type;
	if (fi_color_type == FIC_RGB) {
		gl_color_type = GL_BGR;
	}
	else if (fi_color_type == FIC_RGBALPHA) {
		gl_color_type = GL_RGBA;
	}
	else if (fi_color_type == FIC_MINISBLACK) {
		gl_color_type = GL_LUMINANCE;
	}
	else {
		std::cout << "Unsupported color type: " << t->mFilename.C_Str() << std::endl;
		FreeImage_Unload(dib);
		return false;
	}

	glGenTextures(1, &texture_ID);
	// Bind to texture unit and increment counter
	texture_unit = unit_counter++;
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_color_type, GL_UNSIGNED_BYTE, data);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	FreeImage_Unload(dib);
	return true;
}

bool Texture::loadSkybox(std::string path) {
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* dib(0);
	unsigned char* data(0);
	unsigned int width(0), height(0);

	format = FreeImage_GetFileType(path.c_str(), 0);

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(path.c_str());
	}
	if (format == FIF_UNKNOWN) {
		std::cout << "Uknown texture format: " << path << std::endl;
		return false;
	}

	if (FreeImage_FIFSupportsReading(format)) {
		dib = FreeImage_Load(format, path.c_str());
	}
	if (!dib) {
		std::cout << "Could not load texture: " << path << std::endl;
		return false;
	}


	data = FreeImage_GetBits(dib);

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);


	GLenum gl_color_type;
	int bytes_per_color;
	if (format == FIF_BMP || format == FIF_JPEG) {
		gl_color_type = GL_BGR;
		bytes_per_color = 3;
	}
	else if (format == FIF_PNG) {
		gl_color_type = GL_BGRA;
		bytes_per_color = 4;
	}
	else {
		std::cout << "Unsupported color type: " << path << std::endl;
		FreeImage_Unload(dib);
		return false;
	}

	int face_size = width / 4;

	glGenTextures(1, &texture_ID);
	// Bind to texture unit and increment counter
	texture_unit = unit_counter++;
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_ID);
	int offsets[6][2] = { {2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {3, 1} };
	unsigned char* face = new unsigned char[face_size * face_size * bytes_per_color];
	for (int i = 0; i < 6; i++) {
		int offset_x = offsets[i][0] * face_size;
		int offset_y = offsets[i][1] * face_size;

		for (int j = 0; j < face_size; j++) {
			unsigned int memory_offset = (offset_y * width + j * width + offset_x) * bytes_per_color;
			std::copy(data + memory_offset, data + memory_offset + face_size * bytes_per_color, face + face_size * j * bytes_per_color);
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, face_size, face_size, 0, gl_color_type, GL_UNSIGNED_BYTE, face
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return true;
}

GLuint Texture::reserveUnit() {
	unit_counter++;
	return unit_counter - 1;
}
