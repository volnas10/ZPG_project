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

#include "Shader.h"
#include "Program.h"
#include "Light.h"
#include "Texture.h"

#include "Scene.h"

using namespace Assimp;

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

object::Object* Scene::parseObject(const aiScene* scene, aiString path) {
	std::vector<Texture*> memory_textures;
	std::vector<std::pair<object::Material, std::vector<Texture*>>> materials;
	object::Object* object = new object::Object();

	// Load textures from memory
	for (unsigned int t_index = 0; t_index < scene->mNumTextures; t_index++) {
		aiTexture* texture = scene->mTextures[t_index];
		Texture* t = new Texture();
		t->loadFromMemory(texture);
		memory_textures.push_back(t);
	}

	// Parse materials
	for (unsigned int m_index = 0; m_index < scene->mNumMaterials; m_index++) {
		aiMaterial* material = scene->mMaterials[m_index];

		// Extract properties from assimp material
		object::Material my_material;
		std::vector<Texture*> textures;
		aiColor3D color;
		float val;

		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			material->Get(AI_MATKEY_OPACITY, val);
			my_material.diffuse_color = glm::vec4(color.r, color.g, color.b, val);
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
			my_material.transparent_color = glm::vec4(color.r, color.g, color.b, 0);
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
		for (int i = 0; i <= 21; i++) {
			aiTextureType type = aiTextureType(i);
			unsigned int count = material->GetTextureCount(type);
			if (count == 0) continue;
			aiString texture_name;
			material->Get(AI_MATKEY_TEXTURE(type, 0), texture_name);

			Texture* t;
			// Embedded texture
			if (texture_name.C_Str()[0] == '*') {
				t = memory_textures[atoi(texture_name.C_Str() + 1)];
			}
			// Texture from file
			else {
				aiString full_path = path;
				full_path.Append(texture_name.C_Str());
				stringutil::replaceChar(full_path.data, '\\', '/');
				t = new Texture();
				t->load(full_path.C_Str());
			}

			if (t == nullptr) continue;

			if (type == aiTextureType_DIFFUSE) {
				t->setType(Texture::DIFFUSE);
			}
			else if (type == aiTextureType_HEIGHT) {
				t->setType(Texture::NORMAL);
			}
			// With one testing model opacity is stored under normals so... I'll use it
			else if (type == aiTextureType_NORMALS) {
				t->setType(Texture::OPACITY);
			}
			textures.push_back(t);
		}

		materials.push_back(std::make_pair(my_material, textures));
	}

	// Parse meshes
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<unsigned int> indices;

		for (unsigned int v_index = 0; v_index < mesh->mNumVertices; v_index++) {
			aiVector3D vertex = mesh->mVertices[v_index];
			aiVector3D normal = mesh->mNormals[v_index];
			aiVector3D tangent = mesh->mTangents[v_index];
			aiVector3D bitangent = mesh->mBitangents[v_index];
			vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
			normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
			tangents.push_back(glm::vec3(tangent.x, tangent.y, tangent.z));
			bitangents.push_back(glm::vec3(bitangent.x, bitangent.y, bitangent.z));
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
		object->addMesh(vertices, normals, uvs, tangents, bitangents, indices, materials[mesh->mMaterialIndex].first, materials[mesh->mMaterialIndex].second);
	}

	return object;
}

Scene::Scene(std::string name) {
	this->name = name;
}

Scene::~Scene() {
	delete camera;
	for (AbstractRenderer* r : other_renderers) {
		delete r;
	}
	for (Program* p : programs) {
		delete p;
	}
	delete lights;
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
	std::vector<Shader> shaders;
	lights = new LightCollection();
	std::vector<glm::vec3> obstacles;
	
	Importer importer;
	std::string line;
	while (true) {
		std::getline(description, line);
		if (description.eof()) break;
		// Load transformation
		if (line.find("Transformation") == 0) {
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
					obstacles.push_back(glm::vec3(x, y, z));
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
		// Load model
		else if (line.find("Model") == 0) {
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "name") {
					std::string name;
					sstream >> name;

					const aiScene* scene = importer.ReadFile((path + name).c_str(),
						aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace);
					if (!scene) {
						description.close();
						std::cout << "Cannot load object " << name << std::endl;
						std::cout << importer.GetErrorString() << std::endl;
						continue;
					}

					std::string model_path = path;
					size_t last_slash = name.rfind('/');
					if (last_slash != std::string::npos) {
						path += name.substr(0, last_slash);
					}

					object::Object* obj = parseObject(scene, aiString(model_path));
					models.push_back(obj);
				}
				std::getline(description, line);
			}
		}
		// Load object
		else if (line.find("Object") == 0) {
			int model_index = 0;
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
				}
				else  if (key == "model") {
					sstream >> model_index;
				}

				std::getline(description, line);
			}

			std::vector<trans::Transformation*> object_transformations;
			for (int idx : trans_indices) {
				object_transformations.push_back(transformations[idx]);
			}
			if (object_transformations.size() == 0) {
				object_transformations.push_back(new trans::Transformation());
			}
			objects.push_back(std::make_pair(models[model_index], object_transformations));
		}
		// Make object randomized
		else if (line.find("RandomizeObject") == 0) {
			int model_index = 0;
			int count = 1;
			float distance = 0;
			bool use_obstacles = false;
			glm::vec3 bound1;
			glm::vec3 bound2;
			trans::Transformation* base_trans = nullptr;
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "base") {
					int index;
					sstream >> index;
					base_trans = transformations[index];
				}
				else if (key == "model") {
					sstream >> model_index;
				}
				else if (key == "count") {
					sstream >> count;
				}
				else if (key == "distance") {
					sstream >> distance;
				}
				else if (key == "obstacles") {
					std::string val;
					sstream >> val;
					if (val == "true") {
						use_obstacles = true;
					}
				}
				else if (key == "bounds") {
					sstream >> bound1.x >> bound1.y >> bound1.z >> bound2.x >> bound2.y >> bound2.z;
				}

				std::getline(description, line);
			}
			trans::TransformationGenerator generator;
			generator.setMinDistance(distance);
			if (use_obstacles) {
				generator.addObstacles(obstacles);
			}

			std::vector<trans::Transformation*> random_transformations;

			random_transformations = generator.generateTransformations(count, bound1, bound2, base_trans);

			std::vector<glm::vec3> new_obstacles = generator.getObstacles();
			obstacles.reserve(obstacles.size() + new_obstacles.size());
			obstacles.insert(obstacles.end(), new_obstacles.begin(), new_obstacles.end());

			objects.push_back(std::make_pair(models[model_index], random_transformations));
		}

		// Load light
		else if (line.find("Light") == 0) {
			glm::vec3 position(.0f, .0f, .0f);
			glm::vec3 direction(.0f, .0f, .0f);
			glm::vec3 color(1.0f, 1.0f, 1.0f);
			glm::vec3 attenuation(1.0f, 1.0f, 1.0f);
			float angle = 30.0f;
			std::string type = "point";

			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "position") {
					sstream >> position.x >> position.y >> position.z;
				}
				if (key == "direction") {
					sstream >> direction.x >> direction.y >> direction.z;
				}
				else if (key == "color") {
					sstream >> color.x >> color.y >> color.z;
				}
				else if (key == "angle") {
					sstream >> angle;
				}
				else if (key == "type") {
					sstream >> type;
				}
				else if (key == "attenuation") {
					sstream >> attenuation.x >> attenuation.y >> attenuation.z;
					if (attenuation.x == 0) {
						attenuation.x = 1;
					}
				}
				std::getline(description, line);
			}

			Light* light = new Light(color);
			if (type == "point") {
				light->makePoint(position, attenuation);
			}
			else if (type == "directional") {
				light->makeDirectional(direction);
			}
			else if (type == "spotlight") {
				light->makeSpotlight(position, direction, attenuation, angle);
			}
			else if (type == "flashlight") {
				light->makeFlashlight(attenuation, angle);
			}
			lights->addLight(light);
		}

		// Load shader
		else if (line.find("Shader") == 0) {
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
		else if (line.find("Program") == 0) {
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
			Program* pr = new Program(program_shaders);
			programs.push_back(pr);
			lights->subscribe(pr);
		}
		// Load renderers
		else if (line.find("Renderer") == 0) {
			// Load parameters
			std::getline(description, line);
			std::vector<int> object_indices;
			int program_index;
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
				std::getline(description, line);
			}

			RenderingGroup* rendering_group = new RenderingGroup(programs[program_index]);
			for (int object_index : object_indices) {
				auto pair = objects[object_index];
				rendering_group->addAllObjectTransformations(pair.first, pair.second);
			}
			rendering_groups.push_back(rendering_group);
		}
		// Load skybox
		else if (line.find("Skybox") == 0) {
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
					Texture* skybox = new Texture();
					skybox->loadSkybox(path + value);
					skybox->setType(Texture::SKYBOX);
					SkyboxRenderer* renderer = new SkyboxRenderer(skybox_program, skybox);
					programs.push_back(skybox_program);
					other_renderers.push_back(renderer);
				}
				std::getline(description, line);
			}
		}

		// Load camera
		else if (line.find("Camera") == 0) {
			float fov = 70.f;
			glm::vec3 position(0, 0, -0.01);
			glm::vec3 look_at(0);
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "position") {
					sstream >> position.x >> position.y >> position.z;
				}
				else if (key == "fov") {
					sstream >> fov;
				}
				else if (key == "lookat") {
					sstream >> look_at.x >> look_at.y >> look_at.z;
				}

				std::getline(description, line);
			}

			glm::vec3 camera_direction = glm::normalize(look_at - position);
			float horizontal_angle = 0;
			float vertical_angle = 0;
			// Calculate horizontal angle
			if (camera_direction.x == 0) {
				horizontal_angle = camera_direction.z >= 0 ? 0 : PI;
			}
			else if (camera_direction.z == 0) {
				horizontal_angle = camera_direction.x >= 0 ? PI / 2 : PI * 1.5;
			}
			else {
				horizontal_angle = glm::atan(camera_direction.x / camera_direction.z);
				if (camera_direction.z < 0) {
					horizontal_angle += camera_direction.x > 0 ? PI : -PI;
				}
			}
			// Calculate vertical angle
			float adjacent = std::sqrt(std::pow(camera_direction.x, 2) + std::pow(camera_direction.z, 2));
			if (adjacent == 0) {
				vertical_angle = camera_direction.y >= 0 ? -PI / 2 : PI / 2;
			}
			else if (camera_direction.y != 0) {
				vertical_angle = glm::atan(camera_direction.y / adjacent);
			}

			// Aspect ratio will get adjusted in window
			camera = new Camera(position, fov, horizontal_angle, vertical_angle, 1.77777);
		}

		// Load floor
		else if (line.find("Floor") == 0) {
			Texture* texture = new Texture();
			texture->setType(Texture::DIFFUSE);
			float size = 1.0;
			float dimension = 1;
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "diffuse_texture") {
					std::string value;
					sstream >> value;
					texture->load((path + value).c_str());
				}
				else if (key == "size") {
					sstream >> size;
				}
				else if (key == "dimension") {
					sstream >> dimension;
				}
				std::getline(description, line);
			}
			std::vector<Shader> shaders;
			shaders.push_back(Shader("FloorVertexShader.glsl", GL_VERTEX_SHADER));
			shaders.push_back(Shader("FloorFragmentShader.glsl", GL_FRAGMENT_SHADER));
			Program* floor_program = new Program(shaders);
			lights->subscribe(floor_program);
			FloorRenderer* renderer = new FloorRenderer(floor_program, size, dimension, texture);

			programs.push_back(floor_program);
			other_renderers.push_back(renderer);
		}
	}

	description.close();
	return true;
}

std::vector<object::Object*> Scene::getObjects()
{
	return models;
}

std::vector<RenderingGroup*> Scene::getRenderingGroups()
{
	return rendering_groups;
}

std::vector<AbstractRenderer*> Scene::getRenderers()
{
	return other_renderers;
}

std::vector<Program*> Scene::getPrograms() {
	return programs;
}

Camera* Scene::getCamera() {
	return camera;
}

LightCollection* Scene::getLights() {
	return lights;
}

void Scene::moveObjects(double delta_time) {
	transformation_controller.move(delta_time);
}

