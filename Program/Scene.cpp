#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Scene.h"

using namespace Assimp;


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
		if (line.empty()) break;

		if (line.find("Object") != std::string::npos) {
			// Load object
			std::getline(description, line);
			while (line.find("}") == std::string::npos) {
				std::stringstream sstream(line);
				std::string key, value;
				sstream >> key >> value;
				if (key == "model:") {
					const aiScene* scene = importer.ReadFile((path + value).c_str(), aiProcess_Triangulate |
						aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
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

