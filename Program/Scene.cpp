#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <freeimage.h>


#include "Shader.h"
#include "Program.h"

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

void parseArray(std::string line, std::vector<int>* storage) {
	size_t start = line.find("[");
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
	std::vector<glm::mat3> lights;
	std::vector<Shader> shaders;
	std::vector<std::pair<Object*, std::vector<trans::Transformation*>>> objects;
	
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
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "transformations") {
					parseArray(line, &trans_indices);
					std::getline(description, line);
					continue;
				}
				std::string value;
				sstream >> value;
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
			std::vector<trans::Transformation*> object_transformations;
			for (int idx : trans_indices) {
				object_transformations.push_back(transformations[idx]);
			}
			objects.push_back(std::make_pair(obj, object_transformations));
		}
		// Load light
		else if (line.find("Light") != std::string::npos) {
			glm::vec3 light_pos(.0f, .0f, .0f);
			glm::vec3 light_color(1.0f, 1.0f, 1.0f);
			float light_power = 50.0f;
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "position") {
					sstream >> light_pos.x >> light_pos.y >> light_pos.z;
				}
				else if (key == "color") {
					sstream >> light_color.x >> light_color.x >> light_color.z;
				}
				else if (key == "power") {
					sstream >> light_power;
				}
				std::getline(description, line);
			}
			glm::mat3 light;
			light[0] = light_pos;
			light[1] = light_color;
			light[2][0] = light_power;
			lights.push_back(light);
		}

		// Load shader
		else if (line.find("Shader") != std::string::npos) {
			std::map<std::string, std::string> values;
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key, value;
				sstream >> key >> value;
				key.pop_back();
				values[key] = value;
			
				std::getline(description, line);
			}
			GLenum shader_type;
			if (values["type"] == "vertex") {
				shader_type = GL_VERTEX_SHADER;
			}
			else if (values["type"] == "fragment") {
				shader_type = GL_FRAGMENT_SHADER;
			}
			Shader shader(values["name"], shader_type);
			shaders.push_back(shader);
		}

		// Load program
		else if (line.find("Program") != std::string::npos) {
			// Load parameters
			std::getline(description, line);
			std::vector<int> shader_indices;
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "shaders") {
					parseArray(line, &shader_indices);
				}

				std::getline(description, line);
			}
			std::vector<Shader> program_shaders;
			for (int idx : shader_indices) {
				program_shaders.push_back(shaders[idx]);
			}
			programs.push_back(new Program(program_shaders));
		}
		// Load renderers
		else if (line.find("Renderer") != std::string::npos) {
			// Load parameters
			std::getline(description, line);
			std::vector<int> object_indices;
			int program_index;
			int light_index = -1;
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "objects") {
					parseArray(line, &object_indices);
				}
				else if (key == "program") {
					sstream >> program_index;
				}
				else if (key == "light") {
					sstream >> light_index;
				}
				std::getline(description, line);
			}
			ObjectRenderer* renderer = new ObjectRenderer(programs[program_index]);
			for (int object_index : object_indices) {
				auto pair = objects[object_index];
				renderer->addObject(pair.first, pair.second);
			}
			if (light_index > -1) {
				renderer->setLight(lights[light_index]);
			}
			else {
				renderer->setLight(glm::mat3(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 50.0f, .0f, .0f));
			}
			renderers.push_back(renderer);
		}
	}

	description.close();
	return true;
}

std::vector<ObjectRenderer*> Scene::getRenderers() {
	return renderers;
}

std::vector<Program*> Scene::getPrograms() {
	return programs;
}

