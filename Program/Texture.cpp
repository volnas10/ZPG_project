#include <FreeImage.h>
#include <string>
#include <iostream>
#include "BS_thread_pool.hpp"

#include "Texture.h"
#include "Util.h"


int Texture::unit_counter = 1;

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

void PrecomputeIrradiance(float* irradiance_map, float* data, int width, int height, int map_width, int map_height, int i) {
	int half_height = height / 2;
	for (int j = 0; j < map_width; j++) {
		int center_x = j * (width / map_width) + map_width / 2;
		int center_y = i * (height / map_height) + map_height / 2;

		float irradiance[3] = { 0.0f, 0.0f, 0.0f };

		// Loop over the pixels in the square around the center
		for (int dx = -half_height; dx <= half_height; dx++) {
			for (int dy = -half_height; dy <= half_height; dy++) {
				// Throw away pixels outisde the circle
				if (dx * dx + dy * dy > half_height * half_height) {
					continue;
				}

				// Wrap around the image
				int x = (center_x + dx + width) % width;
				int y = (center_y + dy + height) % height;

				// Get the pixel color
				float* pixel = data + 3 * y * width + 3 * x;
				float color[3] = { pixel[0], pixel[1], pixel[2] };

				// Cosine-weighted importance sampling
				float distance = sqrtf(dx * dx + dy * dy) / half_height;
				float weight = cosf(distance * HALF_PI);

				irradiance[0] += color[0] * weight;
				irradiance[1] += color[1] * weight;
				irradiance[2] += color[2] * weight;
			}
		}

		// Average by the number of samples
		int samples = (int)(PI * (float)(half_height * half_height));
		irradiance[0] /= samples;
		irradiance[1] /= samples;
		irradiance[2] /= samples;

		// Add to the irradiance map
		irradiance_map[3 * (i * map_width + j) + 0] = irradiance[0];
		irradiance_map[3 * (i * map_width + j) + 1] = irradiance[1];
		irradiance_map[3 * (i * map_width + j) + 2] = irradiance[2];
	}
	std::cout << "Row " << i << " done" << std::endl;
}

bool Texture::loadHDRI(const char* filename) {
	FIBITMAP* bitmap = FreeImage_Load(FIF_EXR, filename, EXR_DEFAULT);

	if (!bitmap) {
		std::cerr << "Failed to load image: " << filename << std::endl;
		return 0;
	}

	// Get image width and height
	int width = FreeImage_GetWidth(bitmap);
	int height = FreeImage_GetHeight(bitmap);

	// Make irradiance map
	std::cout << "No irradiance map found, precomputing..." << std::endl;
	float* raw_data = reinterpret_cast<float*>(FreeImage_GetBits(bitmap));
	int map_width = 64;
	int map_height = 32;
	float* irradiance_map = new float[map_width * map_height * 3];

	int threads = std::thread::hardware_concurrency();
	BS::thread_pool pool(threads);
	for (int i = 0; i < map_height; i++) {
		pool.push_task(PrecomputeIrradiance, irradiance_map, raw_data, width, height, map_width, map_height, i);
	}
	
	pool.wait_for_tasks();

	// Save the irradiance map
	FIBITMAP* ir_bitmap = FreeImage_AllocateT(FIT_RGBF, map_width, map_height);
	float* dst = reinterpret_cast<float*>(FreeImage_GetBits(ir_bitmap));
	for (int i = 0; i < map_width * map_height * 3; i++) {
		*dst++ = irradiance_map[i];
	}

	if (!FreeImage_Save(FIF_EXR, ir_bitmap, "../Resources/irradiance_map.exr", EXR_DEFAULT)) {
		// Handle error: failed to save EXR file
		std::cout << "Could not save irradiance map" << std::endl;
		FreeImage_Unload(ir_bitmap);
		delete[] irradiance_map;
		return false;
	}

	FreeImage_Unload(ir_bitmap);
	delete[] irradiance_map;

	// Convert image to 32-bit float format (RGBA)
	FIBITMAP* bitmap32 = FreeImage_ConvertToRGBAF(bitmap);
	FreeImage_Unload(bitmap);

	BYTE* data = FreeImage_GetBits(bitmap32);

	glGenTextures(1, &texture_ID);
	// Bind to texture unit and increment counter
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Free FreeImage data
	FreeImage_Unload(bitmap32);

	texture_type = Type::HDRI;

	return true;
}

GLuint Texture::reserveUnit() {
	unit_counter++;
	return unit_counter - 1;
}
