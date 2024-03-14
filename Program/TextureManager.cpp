#include <FreeImage.h>
#include <string>
#include <iostream>
#include <random>
#include "BS_thread_pool.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "TextureManager.h"
#include "Util.h"
/*
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
*/

void PrecomputeIrradiance(float* irradiance_map, float* data, int width, int height, int map_width, int map_height, int i) {
	std::mt19937 gen;
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	for (int j = 0; j < map_width; j++) {

		// Convert pixel to vector in space
		float theta = (i + 0.5f) / map_height * PI;
		float phi = (j + 0.5f) / map_width * TWO_PI;

		glm::vec3 normal{
			sinf(theta) * cosf(phi),
			sinf(theta) * sinf(phi),
			cosf(theta)
		};
		normal = glm::normalize(normal);

		float irradiance[3] = { 0.0f, 0.0f, 0.0f };

		glm::vec3 o2 = (abs(normal.x) > abs(normal.z)) ? glm::vec3(normal.y, -normal.x, 0.0f) : glm::vec3(0.0f, normal.z, -normal.y);
		o2 = glm::normalize(o2);
		glm::vec3 o1 = glm::normalize(glm::cross(o2, normal));
		glm::mat3 T = glm::mat3(o1, o2, normal);

		for (int s = 0; s < 100000; s++) {
			// Sample hemisphere
			float ksi1 = dis(gen);
			float ksi2 = dis(gen);

			glm::vec3 sample{
				cosf(TWO_PI * ksi1) * sqrtf(1.0f - ksi2),
				sinf(TWO_PI * ksi1) * sqrtf(1.0f - ksi2),
				sqrtf(ksi2)
			};
			sample = glm::normalize(sample);
			float pdf = sample.z / PI;
			sample = T * sample;

			// Convert back to coordinates on the environment map
			const float theta = acosf(sample.z);
			const float phi = atan2f(sample.y, sample.x) + ((sample.y < 0.0f) ? TWO_PI : 0.0f);

			int x = int((phi / TWO_PI) * width + 0.5f) % width;
			int y = int((theta / PI) * height + 0.5f) % height;

			float costheta = glm::dot(normal, sample);
			irradiance[0] += data[3 * (y * width + x) + 0] * costheta * (1.0f / pdf);
			irradiance[1] += data[3 * (y * width + x) + 1] * costheta * (1.0f / pdf);
			irradiance[2] += data[3 * (y * width + x) + 2] * costheta * (1.0f / pdf);
		}

		// Average by the number of samples
		irradiance[0] /= 100000;
		irradiance[1] /= 100000;
		irradiance[2] /= 100000;

		// Add to the irradiance map
		irradiance_map[3 * (i * map_width + j) + 0] = irradiance[0];
		irradiance_map[3 * (i * map_width + j) + 1] = irradiance[1];
		irradiance_map[3 * (i * map_width + j) + 2] = irradiance[2];
	}
	std::cout << "Row " << i << " done" << std::endl;
}

TextureManager& TextureManager::getInstance() {
	static TextureManager instance;
	return instance;
}

void TextureManager::init() {
	TextureManager& instance = getInstance();
	instance.id_counter = 0;

	glGenBuffers(1, &instance.buffer_ID);
	glBindBuffer(GL_UNIFORM_BUFFER, instance.buffer_ID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TexturePack) * MAX_TEXTURE_BLOCKS, NULL, GL_DYNAMIC_DRAW);
	instance.updated = true;
}

void TextureManager::addEnvMap(const char* filename) {
	TextureManager& instance = getInstance();
	FIBITMAP* bitmap = FreeImage_Load(FIF_EXR, filename, EXR_DEFAULT);

	if (!bitmap) {
		std::cerr << "Failed to load environment map: " << filename << std::endl;
	}

	// Get image width and height
	int width = FreeImage_GetWidth(bitmap);
	int height = FreeImage_GetHeight(bitmap);

	BYTE* data = FreeImage_GetBits(bitmap);

	glGenTextures(1, &instance.env_map);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, instance.env_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Find if irradiance map exists
	std::string path(filename);
	int slash = path.rfind("/");
	path.replace(slash + 1, path.size() - slash, "irradiance_map.exr");
	FIBITMAP* ir_bitmap = FreeImage_Load(FIF_EXR, path.c_str(), EXR_DEFAULT);

	int map_width = 64;
	int map_height = 32;
	float* irradiance_map = new float[map_width * map_height * 3];

	// Load map
	if (!ir_bitmap) {
		std::cout << "No irradiance map found, precomputing..." << std::endl;
		float* raw_data = reinterpret_cast<float*>(FreeImage_GetBits(bitmap));
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

		if (!FreeImage_Save(FIF_EXR, ir_bitmap, path.c_str(), EXR_DEFAULT)) {
			// Handle error: failed to save EXR file
			std::cout << "Could not save irradiance map" << std::endl;
		}

	}

	delete[] irradiance_map;

	data = FreeImage_GetBits(ir_bitmap);

	glGenTextures(1, &instance.env_map);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, instance.irradiance_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, map_width, map_height, 0, GL_RGB, GL_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Free FreeImage data
	FreeImage_Unload(bitmap);
	FreeImage_Unload(ir_bitmap);
}

void TextureManager::addCrosshair(const char* filename) {
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
	}

	if (FreeImage_FIFSupportsReading(format)) {
		dib = FreeImage_Load(format, filename);
	}
	if (!dib) {
		std::cout << "Could not load texture: " << filename << std::endl;
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
	}

	data = FreeImage_GetBits(dib);

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	GLuint crosshair;
	glGenTextures(1, &crosshair);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, crosshair);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_color_type, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	FreeImage_Unload(dib);
}

int TextureManager::addMaterial(aiTexture* t, TextureType type) {
	TextureManager& instance = getInstance();
	instance.texture_packs.push_back(TexturePack());
	int id = instance.id_counter++;
	return addMaterial(id, t, type);
}

int TextureManager::addMaterial(const char* filename, TextureType type) {
	TextureManager& instance = getInstance();
	instance.texture_packs.push_back(TexturePack());
	int id = instance.id_counter++;
	return addMaterial(id, filename, type);
}

int TextureManager::addMaterial(int material_id, aiTexture* t, TextureType type) {
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
		return -1;
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
		return -1;
	}

	GLuint64 handle;
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_color_type, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureHandleARB(texture_id);
	glMakeTextureHandleResidentARB(handle);

	FreeImage_Unload(dib);

	TextureManager& instance = getInstance();
	if (type == TextureType::DIFFUSE) {
		instance.texture_packs[material_id].diffuse = handle;
	}
	else if (type == TextureType::NORMAL) {
		instance.texture_packs[material_id].normal = handle;
	}
	else if (type == TextureType::SPECULAR) {
		instance.texture_packs[material_id].specular = handle;
	}

	return material_id;
}

int TextureManager::addMaterial(int material_id, const char* filename, TextureType type) {
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
		return -1;
	}

	if (FreeImage_FIFSupportsReading(format)) {
		dib = FreeImage_Load(format, filename);
	}
	if (!dib) {
		std::cout << "Could not load texture: " << filename << std::endl;
		return -1;
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
		return -1;
	}

	data = FreeImage_GetBits(dib);

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	GLuint64 handle;
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_color_type, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureHandleARB(texture_id);
	glMakeTextureHandleResidentARB(handle);

	FreeImage_Unload(dib);

	TextureManager& instance = getInstance();
	if (type == TextureType::DIFFUSE) {
		instance.texture_packs[material_id].diffuse = handle;
	}
	else if (type == TextureType::NORMAL) {
		instance.texture_packs[material_id].normal = handle;
	}
	else if (type == TextureType::SPECULAR) {
		instance.texture_packs[material_id].specular = handle;
	}

	return material_id;
}

void TextureManager::bind(int block_idx) {
	TextureManager& instance = getInstance();
	if (instance.texture_packs.size() != 0 && instance.updated) {
		glBindBuffer(GL_UNIFORM_BUFFER, instance.buffer_ID);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, instance.texture_packs.size() * sizeof(TextureManager::TexturePack), &instance.texture_packs[0]);
		instance.updated = false;
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, block_idx, instance.buffer_ID);
}
