#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <freeimage.h>

#include "Scene.h"

using namespace Assimp;

GLuint loadTexture(const char* filename) {
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* dib(0);
	unsigned char* data(0);
	unsigned int width(0), height(0);

	format = FreeImage_GetFileType(filename, 0);

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(filename);
	}
	if (format == FIF_UNKNOWN) {
		std::cout << "Uknown texture format: " << filename << std::endl;
		return 0;
	}

	if (FreeImage_FIFSupportsReading(format)) {
		dib = FreeImage_Load(format, filename);
	}
	if (!dib) {
		std::cout << "Could not load texture: " << filename << std::endl;
		return 0;
	}


	data = FreeImage_GetBits(dib);

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);


	FREE_IMAGE_COLOR_TYPE color_type = FreeImage_GetColorType(dib);
	GLenum gl_color_type;
	if (format == FIF_BMP || format == FIF_JPEG) {
		gl_color_type = GL_BGR;
	}
	else if (format == FIF_PNG) {
		gl_color_type = GL_RGBA;
	}
	else {
		std::cout << "Unsupported color type: " << filename << std::endl;
		FreeImage_Unload(dib);
		return 0;
	}

	GLuint texture_ID;
	glGenTextures(1, &texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, gl_color_type, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	FreeImage_Unload(dib);

	return texture_ID;
}

void parseMultipleTransformations(std::string line, int start, std::vector<int>* storage) {
	int index = 0;
	for (int i = start + 1; i < line.size(); i++) {
		char ch = line[i];
		if (ch >= '0' && ch <= '9') {
			index *= 10;
			index += ch - '0';
		}
		else if (ch == ',' || ch == ']') {
			storage->push_back(index);
			index = 0;
		}
	}
}


Object* Scene::parseObject(const aiScene* scene, aiString path) {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;
	GLuint texture = 0;
	for (unsigned int m_index = 0; m_index < scene->mNumMaterials; m_index++) {
		aiMaterial* material = scene->mMaterials[m_index];
		aiString name = material->GetName();
		unsigned int texture_count = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (texture_count > 0) {
			aiString texture_name;
			material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_name);
			path.Append(texture_name.C_Str());
			
			texture = loadTexture(path.C_Str());
			
		}
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		for (unsigned int v_index = 0; v_index < mesh->mNumVertices; v_index++) {
			aiVector3D vertex = mesh->mVertices[v_index];
			aiVector3D normal = mesh->mNormals[v_index];
			aiVector3D uv = mesh->mTextureCoords[0][v_index];
			vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
			normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
			uvs.push_back(glm::vec2(uv.x, uv.y));
		}
		for (unsigned int f_index = 0; f_index < mesh->mNumFaces; f_index++) {
			aiFace face = mesh->mFaces[f_index];
			for (int idx = 0; idx < 3; idx++) {
				indices.push_back(face.mIndices[idx]);
			}
		}
	}

	return new Object(vertices, normals, uvs, indices, texture);
}

Scene::Scene(std::string name) {
	this->name = name;
}

bool Scene::load() {
	std::string path = "../Resources/" + name + "/";
	std::ifstream description(path + "description.scene");
	if (!description.is_open()) {
		std::cout << "Scene description not found" << std::endl;
		description.close();
		return false;
	}

	std::vector<trans::Transformation*> transformations;
	
	Importer importer;
	std::string line;
	while (true) {
		std::getline(description, line);
		if (description.eof()) break;
		// Load transformation
		if (line.find("Transformation") != std::string::npos) {
			trans::Transformation* transformation = new trans::Transformation();
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();

				if (key == "scale") {
					float x, y, z;
					sstream >> x >> y >> z;
					transformation->scale(x, y, z);
				}
				else if (key == "position") {
					float x, y, z;
					sstream >> x >> y >> z;
					transformation->translate(x, y, z);
				}
				else if (key == "rotation") {
					float x, y, z;
					sstream >> x >> y >> z;
					transformation->rotate(x, y, z);
				}
				else if (key == "previous") {
					int prev;
					sstream >> prev;
					*transformation << *transformations[prev];
				}
				std::getline(description, line);
			}
			// All parameters loaded, save transformation
			transformations.push_back(transformation);
		}
		// Load object
		else if (line.find("Object") != std::string::npos) {
			std::map<std::string, std::string> values;
			std::vector<int> trans_indices;
			values["path"] = "";
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);

				if (line.find("transformation") != std::string::npos) {
					size_t idx = line.find("[");
					// Load only one transformation
					if (idx == std::string::npos) {
						std::string _;
						sstream >> _;
						int t;
						sstream >> t;
						trans_indices.push_back(t);
					}
					// Load multiple transformations
					else {
						parseMultipleTransformations(line, idx, &trans_indices);
					}
					std::getline(description, line);
					continue;
				}
				std::string key, value;
				sstream >> key >> value;
				key.pop_back();
				values[key] = value;
				std::getline(description, line);
			}

			std::string object_path = path + values["path"];
			const aiScene* scene = importer.ReadFile((object_path + values["model"]).c_str(),
				aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

			if (!scene) {
				description.close();
				std::cout << "Cannot load object " << values["model"] << std::endl;
				continue;
			}

			Object* obj = parseObject(scene, aiString(object_path));
			obj->name = values["model"];
			for (int idx : trans_indices) {
				objects[obj].push_back(transformations[idx]);
			}
		}
		else if (line.find("") == 0) {

		}
	}


	description.close();
	return true;
}

std::map<Object*, std::vector<trans::Transformation*>> Scene::getObjects() {
	return objects;
}

