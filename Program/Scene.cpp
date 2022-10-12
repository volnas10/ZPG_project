#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Scene.h"

using namespace Assimp;


void Scene::parseObject(const aiScene* scene) {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		for (int v_index = 0; v_index < mesh->mNumVertices; v_index++) {
			aiVector3D vertex = mesh->mVertices[v_index];
			aiVector3D normal = mesh->mNormals[v_index];
			vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
			normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
		}
		for (int f_index = 0; f_index < mesh->mNumFaces; f_index++) {
			aiFace face = mesh->mFaces[f_index];
			for (int idx = 0; idx < 3; idx++) {
				indices.push_back(face.mIndices[idx]);
			}
		}
	}

	Object* obj = new Object(vertices, normals, indices);
	objects[obj].push_back(new trans::Transformation());
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

	
	Importer importer;
	std::string line;
	while (true) {
		std::getline(description, line);
		if (description.eof()) break;

		if (line.find("Object") != std::string::npos) {
			// Load object
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key, value;
				sstream >> key >> value;
				if (key == "model:") {
					const aiScene* scene = importer.ReadFile((path + value).c_str(),
						aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
					if (!scene) {
						description.close();
						std::cout << "Cannot load object" << std::endl;
						return false;
					}

					parseObject(scene);
					

				}
				

				std::getline(description, line);
			}
		}
		else if (line.find("") == 0) {

		}
	}


	description.close();
	return true;
}

void Scene::save() {
	std::ofstream description("../Resources/" + name + "/description.scene");




	description.close();
}

std::map<Object*, std::vector<trans::Transformation*>> Scene::getObjects() {
	return objects;
}

