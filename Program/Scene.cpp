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
#include "TextureManager.h"
#include "FloorGenerator.h"
#include "BezierCurve.h"

#include "Scene.h"

using namespace Assimp;

object::Object* Scene::parseObject(const aiScene* scene, aiString path) {
	std::vector<object::Material> materials;
	object::Object* object = new object::Object();
	std::vector<aiTexture*> memory_textures;

	// Load textures from memory
	for (unsigned int t_index = 0; t_index < scene->mNumTextures; t_index++) {
		aiTexture* texture = scene->mTextures[t_index];
		memory_textures.push_back(texture);
	}

	// Parse materials
	for (unsigned int m_index = 0; m_index < scene->mNumMaterials; m_index++) {
		aiMaterial* material = scene->mMaterials[m_index];

		// Extract properties from assimp material
		object::Material my_material;
		my_material.texture_id = -1;
		my_material.metallic = 0.0f;
		aiColor3D color;
		float val;

		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			material->Get(AI_MATKEY_OPACITY, val);
			my_material.diffuse_color = glm::vec4(color.r, color.g, color.b, val);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color)) {
			my_material.ambient_color = glm::vec4(color.r, color.g, color.b, 0);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
			my_material.specular_color = glm::vec4(color.r, color.g, color.b, 0);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_REFRACTI, val)) {
			my_material.refraction_index = val;
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, val)) {
			my_material.shininess = val;
			my_material.roughness = 1 - (sqrtf(val) / 30);
		}
		if (AI_SUCCESS == material->Get(AI_MATKEY_METALLIC_FACTOR, val)) {
			my_material.metallic = val;
		}

		glm::vec4 has_textures(0.0f);
		// Check if material has textures
		for (int i = 0; i <= 21; i++) {
			aiTextureType type = aiTextureType(i);
			unsigned int count = material->GetTextureCount(type);
			if (count == 0) continue;

			TextureManager::TextureType t_type;
			switch (type) {
				case aiTextureType_DIFFUSE:
					t_type = TextureManager::DIFFUSE;
					has_textures.x = 1.0f;
					break;
				case aiTextureType_HEIGHT:
					t_type = TextureManager::NORMAL;
					has_textures.y = 1.0f;
					break;
				case aiTextureType_SPECULAR:
					t_type = TextureManager::SPECULAR;
					has_textures.z = 1.0f;
					break;
				case aiTextureType_DIFFUSE_ROUGHNESS: // RMA doesn't have specific key in mtl file so I use roughness
					t_type = TextureManager::RMA;
					has_textures.w = 1.0f;
					break;
			}

			aiString texture_name;
			material->Get(AI_MATKEY_TEXTURE(type, 0), texture_name);

			// Embedded texture
			if (texture_name.C_Str()[0] == '*') {
				aiTexture* t = memory_textures[atoi(texture_name.C_Str() + 1)];
				if (my_material.texture_id < 0) {
					my_material.texture_id = TextureManager::addMaterial(t, t_type);
				}
				else {
					TextureManager::addMaterial(my_material.texture_id, t, t_type);
				}
			}
			// Texture from file
			else {
				aiString full_path = path;
				full_path.Append(texture_name.C_Str());
				stringutil::replaceChar(full_path.data, '\\', '/');

				if (my_material.texture_id < 0) {
					my_material.texture_id = TextureManager::addMaterial(full_path.C_Str(), t_type);
				}
				else {
					TextureManager::addMaterial(my_material.texture_id, full_path.C_Str(), t_type);
				}
			}
		}

		my_material.has_textures = has_textures;
		materials.push_back(my_material);
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
		object->addMesh(vertices, normals, uvs, tangents, bitangents, indices, materials[mesh->mMaterialIndex]);
	}

	return object;
}

std::vector<float> Scene::parseVertices(const aiScene* scene) {
	aiMesh* mesh = scene->mMeshes[0];
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int v_index = 0; v_index < mesh->mNumVertices; v_index++) {
		aiVector3D vertex = mesh->mVertices[v_index];
		vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
	}
	for (unsigned int f_index = 0; f_index < mesh->mNumFaces; f_index++) {
		aiFace face = mesh->mFaces[f_index];
		for (int idx = 0; idx < 3; idx++) {
			indices.push_back(face.mIndices[idx]);
		}
	}

	std::vector<float> vertex_data;
	for (unsigned int index : indices) {
		glm::vec3 vertex = vertices[index];
		vertex_data.push_back(vertex.x);
		vertex_data.push_back(vertex.y);
		vertex_data.push_back(vertex.z);
	}
	return vertex_data;
}

Scene::Scene(std::string name) {
	this->name = name;
	lights = new LightCollection();
	shadow_type = SHADOWS_NONE;
}

Scene::~Scene() {
	delete camera;
	delete objects;
	for (AbstractRenderer* r : pre_renderers) {
		delete r;
	}
	for (Renderer* r : main_renderers) {
		delete r;
	}
	for (AbstractRenderer* r : post_renderers) {
		delete r;
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
	std::vector<BezierCurve*> bezier_curves;
	std::vector<glm::vec3> obstacles;
	objects = new object::ObjectGroup();
	
	Importer importer;
	std::string line;
	while (true) {
		std::getline(description, line);
		if (description.eof()) break;
		// Load transformation
		if (line.find("Transformation") == 0) {
			trans::Transformation* transformation = new trans::Transformation();
			bool transformed = false;
			int curve_idx = -1;
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
					transformed = true;
				}
				else if (key == "position") {
					float x, y, z;
					sstream >> x >> y >> z;
					t_leaf = transformation->translate(x, y, z);
					obstacles.push_back(glm::vec3(x, y, z));
					transformed = true;
				}
				else if (key == "rotation") {
					float x, y, z;
					sstream >> x >> y >> z;
					t_leaf = transformation->rotate(x / 57.29f, y / 57.29f, z / 57.29f); // (x / 180) * pi = x / 57.29
					transformed = true;
				}
				else if (key == "transformations") {
					std::vector<int> trans_indices;
					stringutil::parseArray(line, &trans_indices);
					for (int i : trans_indices) {
						transformation->addTransformation(transformations[i]);
						//*transformation << *transformations[i];
					}
					transformed = true;
				}
				else if (key == "curve") {
					sstream >> curve_idx;
				}
				std::getline(description, line);

				if (t_leaf != nullptr && line.find("change") != std::string::npos) {
					sstream = std::stringstream(line);
					glm::vec3 change(0.0);
					sstream >> key >> change.x >> change.y >> change.z;
					transformation_controller.addChange(t_leaf, change);
				}

				if (!transformed && curve_idx >= 0) {
					delete transformation;
					transformation = bezier_curves[curve_idx]->getTransformation();
				}
			}
			// All parameters loaded, save transformation
			transformations.push_back(transformation);
		}
		// Load curve
		else if (line.find("Curve") == 0) {
			std::vector<glm::vec3> points;
			int order = 1;
			float speed = 0.1;
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "points") {
					stringutil::parsePointArray(line, &points);
				}
				else if (key == "speed") {
					sstream >> speed;
				}
				else if (key == "order") {
					sstream >> order;
				}
				std::getline(description, line);

			}
			// All parameters loaded, save transformation
			BezierCurve* bc = new BezierCurve(points, order, speed);
			bezier_curves.push_back(bc);
			transformation_controller.addCurve(bc);
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
					std::string obj_name;
					sstream >> obj_name;

					const aiScene* scene = importer.ReadFile((path + obj_name).c_str(),
						aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace);
					if (!scene) {
						description.close();
						std::cout << "Cannot load object " << name << std::endl;
						std::cout << importer.GetErrorString() << std::endl;
						continue;
					}

					std::string model_path = path;
					size_t last_slash = obj_name.rfind('/');
					if (last_slash != std::string::npos) {
						model_path += obj_name.substr(0, last_slash + 1);
					}

					object::Object* obj = parseObject(scene, aiString(model_path));
					obj->name = name;
					models.push_back(obj);
				}
				std::getline(description, line);
			}
		}
		// Load object
		else if (line.find("Object") == 0) {
			int model_index = 0;
			std::vector<int> trans_indices;
			trans::Transformation* default_trans = nullptr;
			// Load parameters
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key;
				sstream >> key;
				key.pop_back();
				if (key == "transformations") {
					stringutil::parseArray(line, &trans_indices);
				}
				if (key == "default_transformation") {
					int idx;
					sstream >> idx;
					default_trans = transformations[idx];
				}
				else  if (key == "model") {
					sstream >> model_index;
				}

				std::getline(description, line);
			}

			std::pair<trans::Transformation*, std::vector<trans::Transformation*>> object_transformations;
			object_transformations.first = default_trans;
			for (int idx : trans_indices) {
				object_transformations.second.push_back(transformations[idx]);
			}
			if (object_transformations.second.size() == 0) {
				object_transformations.second.push_back(new trans::Transformation());
			}
			objects->addAllObjectTransformations(models[model_index], object_transformations);
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

			std::pair<trans::Transformation*, std::vector<trans::Transformation*>> random_transformations;

			random_transformations.first = base_trans;
			random_transformations.second = generator.generateTransformations(count, bound1, bound2);

			std::vector<glm::vec3> new_obstacles = generator.getObstacles();
			obstacles.reserve(obstacles.size() + new_obstacles.size());
			obstacles.insert(obstacles.end(), new_obstacles.begin(), new_obstacles.end());

			objects->addAllObjectTransformations(models[model_index], random_transformations);
		}

		// Load light
		else if (line.find("Light") == 0) {
			glm::vec3 position(.0f, .0f, .0f);
			glm::vec3 direction(.0f, .0f, .0f);
			glm::vec3 attenuation(1.0f, 1.0f, 1.0f);
			trans::Transformation* transformation = nullptr;
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
				else if (key == "transformation") {
					int index;
					sstream >> index;
					transformation = transformations[index];
				}
				std::getline(description, line);
			}

			Light* light = new Light();
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
			if (transformation != nullptr) {
				light->setTransformation(transformation);
			}
			lights->addLight(light);
		}
		/* Load skybox
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
					other_renderers.insert(other_renderers.begin(), renderer);
				}
				std::getline(description, line);
			}
		}
		*/
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

			camera = new Camera(position, fov, horizontal_angle, vertical_angle);
		}

		/* Load floor
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
			std::vector<Texture*> textures;
			textures.push_back(texture);
			FloorGenerator generator(dimension, size, textures);
			std::vector<trans::Transformation*> tmp;
			tmp.push_back(new trans::Transformation());
			object::Object* obj = generator.generate();
			models.push_back(obj);
			objects.push_back(std::make_pair(obj, std::make_pair(nullptr, tmp)));
		}
		*/
		// Load settings for PG2
		else if (line.find("PG2Settings") == 0) {;
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key, value;
				sstream >> key >> value;
				key.pop_back();
				if (key == "environment") {
					TextureManager::addEnvMap((path + value).c_str());

					// Load ico sphere
					const aiScene* scene = importer.ReadFile("../Resources/ico_sphere.obj", NULL);
					std::vector<float> sphere = parseVertices(scene);

					EnvMapRenderer* renderer = new EnvMapRenderer(sphere);
					pre_renderers.push_back(renderer);
				}

				if (key == "shadows") {
					if (value == "none") {
						shadow_type = SHADOWS_NONE;
					}
					else if (value == "map") {
						shadow_type = SHADOWS_MAP;
					}
					else if (value == "stencil") {
						shadow_type = SHADOWS_STENCIL;
					}
				}

				std::getline(description, line);
			}
		}
	}

	description.close();

	if (camera == nullptr) {
		camera = new Camera(glm::vec3(0), 70, 0, 0);
	}
	window_subscribers.push_back(camera);

	// Finally crerate rendering pipeline

	if (shadow_type != SHADOWS_STENCIL) {
		Shader vertex_shader("CompleteVertexShader.glsl", GL_VERTEX_SHADER);
		Shader fragment_shader("CompleteFragmentShader.glsl", GL_FRAGMENT_SHADER);
		Program* program = new Program({ vertex_shader, fragment_shader });
		camera->subscribe(program);
		main_renderers.push_back(new Renderer(program));
	}

	// Post-render
	CrosshairRenderer* crosshair_renderer = new CrosshairRenderer();
	post_renderers.push_back(crosshair_renderer);
	window_subscribers.push_back(crosshair_renderer);

	camera->subscribe(lights);
	return true;
}

std::vector<object::Object*> Scene::getObjects()
{
	return models;
}

object::ObjectGroup* Scene::getObjectGroup() {
	return objects;
}

RENDERERS Scene::getRenderers() {
	return RENDERERS{ pre_renderers, main_renderers, post_renderers };
}

std::vector<WindowSizeSubscriber*> Scene::getWindowSubscribers() {
	return window_subscribers;
}

Camera* Scene::getCamera() {
	return camera;
}

LightCollection* Scene::getLights() {
	return lights;
}

unsigned int Scene::getShadowType() {
	return shadow_type;
}

void Scene::moveObjects(double delta_time) {
	transformation_controller.move(delta_time);
}

