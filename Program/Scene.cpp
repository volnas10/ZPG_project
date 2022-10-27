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
	std::cout << "Loading texture from file: " << filename << std::endl;

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

GLuint loadTextureFromMemory(aiTexture* t) {
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
		return 0;
	}

	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	data = FreeImage_GetBits(dib);

	GLenum gl_color_type;
	if (format == FIF_BMP || format == FIF_JPEG) {
		gl_color_type = GL_BGR;
	}
	else if (format == FIF_PNG) {
		gl_color_type = GL_RGBA;
	}
	else {
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

GLuint loadSkybox(std::string path) {
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
		return 0;
	}

	if (FreeImage_FIFSupportsReading(format)) {
		dib = FreeImage_Load(format, path.c_str());
	}
	if (!dib) {
		std::cout << "Could not load texture: " << path << std::endl;
		return 0;
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
		return 0;
	}

	int face_size = width / 4;
	GLuint texture_ID;
	glGenTextures(1, &texture_ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_ID);
	int offsets[6][2] = { {2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {3, 1}};
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

void nodeBuilder(aiNode* node, trans::Transformation* parent_trans,
	std::map<unsigned int, std::vector<trans::Transformation*>>* mesh_transformations) {

	if (node->mNumChildren == 0 && node->mNumMeshes == 0) return;
	aiMatrix4x4 transformation = node->mTransformation;
	glm::mat4 matrix(
		transformation.a1, transformation.a2, transformation.a3, transformation.a4,
		transformation.b1, transformation.b2, transformation.b3, transformation.b4,
		transformation.c1, transformation.c2, transformation.c3, transformation.c4,
		transformation.d1, transformation.d2, transformation.d3, transformation.d4);

	trans::Transformation* t = new trans::Transformation();
	t->addMatrix(matrix);
	if (parent_trans != nullptr) {
		*t << *parent_trans;
	}
	for (int i = 0; i < node->mNumMeshes; i++) {
		(*mesh_transformations)[node->mMeshes[i]].push_back(t);
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		nodeBuilder(node->mChildren[i], t, mesh_transformations);
	}
}

object::Object* Scene::parseObject(const aiScene* scene, aiString path) {
	std::vector<GLuint> textures;
	std::vector<object::Material> materials;
	object::Object* object = new object::Object();

	// Load tetxures from memory
	for (unsigned int t_index = 0; t_index < scene->mNumTextures; t_index++) {
		aiTexture* texture = scene->mTextures[t_index];
		textures.push_back(loadTextureFromMemory(texture));
	}

	// Parse materials
	for (unsigned int m_index = 0; m_index < scene->mNumMaterials; m_index++) {
		aiMaterial* material = scene->mMaterials[m_index];

		// Extract properties from assimp material
		object::Material my_material;
		aiColor3D color;
		float val;

		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			my_material.diffuse_color = glm::vec4(color.r, color.g, color.b, -1);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color)) {
			my_material.ambient_color = glm::vec4(color.r, color.g, color.b, 0);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, color)) {
			my_material.emissive_color = glm::vec4(color.r, color.g, color.b, 0);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
			my_material.specular_color = glm::vec4(color.r, color.g, color.b, 0);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_TRANSPARENT, color)) {
			val = 1;
			material->Get(AI_MATKEY_OPACITY, val);
			my_material.transparent_color = glm::vec4(color.r, color.g, color.b, val);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_REFLECTIVE, color)) {
			val = 0;
			material->Get(AI_MATKEY_REFLECTIVITY, val);
			my_material.reflective_color = glm::vec4(color.r, color.g, color.b, val);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_REFRACTI, val)) {
			my_material.refraction_index = val;
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, val)) {
			my_material.shininess = val;
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS_STRENGTH, val)) {
			my_material.shininess_strength = val;
		}

		// Check if material has textures
		unsigned int texture_count = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (texture_count > 0) {
			aiString texture_name;
			material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_name);
			// Embedded texture
			if (texture_name.C_Str()[0] == '*') {
				my_material.diffuse_color.a = textures[atoi(texture_name.C_Str() + 1)];
			}
			// Texture from file
			else {
				aiString full_path = path;
				full_path.Append(texture_name.C_Str());
				GLuint id = loadTexture(full_path.C_Str());
				if (id > 0) {
					textures.push_back(id);
					my_material.diffuse_color.a = textures[textures.size() - 1];
				}
			}
		}

		materials.push_back(my_material);
	}

	// Parse meshes
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<unsigned int> indices;

		for (unsigned int v_index = 0; v_index < mesh->mNumVertices; v_index++) {
			aiVector3D vertex = mesh->mVertices[v_index];
			aiVector3D normal = mesh->mNormals[v_index];
			vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
			normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
		}
		if (mesh->HasTextureCoords(0)) {
			for (unsigned int uv_index = 0; uv_index < mesh->mNumVertices; uv_index++) {
				aiVector3D uv = mesh->mTextureCoords[0][uv_index];
				uvs.push_back(glm::vec2(uv.x, uv.y));
			}
		}

		for (unsigned int f_index = 0; f_index < mesh->mNumFaces; f_index++) {
			aiFace face = mesh->mFaces[f_index];
			for (int idx = 0; idx < 3; idx++) {
				indices.push_back(face.mIndices[idx]);
			}
		}
		object->addMesh(vertices, normals, uvs, indices, materials[mesh->mMaterialIndex]);
	}

	// Build the object from meshes and transformations
	aiNode* root_node = scene->mRootNode;

	std::map<unsigned int, std::vector<trans::Transformation*>> mesh_transformations;
	nodeBuilder(root_node, nullptr, &mesh_transformations);
	for (auto pair : mesh_transformations) {
		object->transformMesh(pair.first, pair.second);
	}

	return object;
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
	std::vector<std::pair<object::Object*, std::vector<trans::Transformation*>>> objects;
	
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
				trans::TransformationLeaf* t_leaf = nullptr;
				if (key == "scale") {
					float x, y, z;
					sstream >> x >> y >> z;
					t_leaf = transformation->scale(x, y, z);
				}
				else if (key == "position") {
					float x, y, z;
					sstream >> x >> y >> z;
					t_leaf = transformation->translate(x, y, z);
				}
				else if (key == "rotation") {
					float x, y, z;
					sstream >> x >> y >> z;
					t_leaf = transformation->rotate(x / 57.29f, y / 57.29f, z / 57.29f); // (x / 180) * pi = x / 57.29
				}
				else if (key == "transformations") {
					std::vector<int> trans_indices;
					parseArray(line, &trans_indices);
					for (int i : trans_indices) {
						*transformation << *transformations[i];
					}
				}
				std::getline(description, line);

				if (t_leaf != nullptr && line.find("change") != std::string::npos) {
					sstream = std::stringstream(line);
					glm::vec3 change(0.0);
					sstream >> key >> change.x >> change.y >> change.z;
					transformation_controller.addChange(t_leaf, change);
				}
			}
			// All parameters loaded, save transformation
			transformations.push_back(transformation);
		}
		// Load object
		else if (line.find("Object") != std::string::npos) {
			std::map<std::string, std::string> values;
			std::vector<int> trans_indices;
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
				aiProcess_Triangulate | aiProcess_PreTransformVertices);
			if (!scene) {
				description.close();
				std::cout << "Cannot load object " << values["model"] << std::endl;
				std::cout << importer.GetErrorString() << std::endl;
				continue;
			}

			object::Object* obj = parseObject(scene, aiString(object_path));
			obj->name = values["model"];
			std::vector<trans::Transformation*> object_transformations;
			for (int idx : trans_indices) {
				object_transformations.push_back(transformations[idx]);
			}
			if (object_transformations.size() == 0) {
				object_transformations.push_back(new trans::Transformation());
			}
			objects.push_back(std::make_pair(obj, object_transformations));

			// Load lights from scene if present
			//if (scene->HasLights()) {
			//	for (int i = 0; i < scene->mNumLights; i++) {
			//		aiLight* light = scene->mLights[i];
			//		aiVector3D pos = light->mPosition;
			//		aiColor3D color = light->mColorDiffuse;
			//		float power = light->mAttenuationConstant;
			//		glm::mat3 light_matrix(0.0);
			//		light_matrix[0] = glm::vec3(pos.x, pos.y, pos.z);
			//		light_matrix[1] = glm::vec3(color.r, color.g, color.b);
			//		light_matrix[2][0] = power;
			//		lights.push_back(light_matrix);
			//	}
			//}
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
			glm::mat3 light(0.0);
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
		// Load skybox
		else if (line.find("Skybox") != std::string::npos) {
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key, value;
				sstream >> key >> value;
				key.pop_back();
				if (key == "name") {
					std::vector<Shader> shaders;
					shaders.push_back(Shader("SkyboxVertexShader.glsl", GL_VERTEX_SHADER));
					shaders.push_back(Shader("SkyboxFragmentShader.glsl", GL_FRAGMENT_SHADER));
					Program* skybox_program = new Program(shaders);
					GLuint skybox_ID = loadSkybox(path + value);
					SkyboxRenderer* renderer = new SkyboxRenderer(skybox_program, skybox_ID);
					programs.push_back(skybox_program);
					renderers.insert(renderers.begin(), 1, renderer);
				}
				std::getline(description, line);
			}
		}
	}

	description.close();
	return true;
}

std::vector<AbstractRenderer*> Scene::getRenderers() {
	return renderers;
}

std::vector<Program*> Scene::getPrograms() {
	return programs;
}

void Scene::moveObjects(double delta_time) {
	transformation_controller.move(delta_time);
}

