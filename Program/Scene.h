#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <assimp/scene.h>
#include <map>
#include "Object.h"
#include "Util.h"

class Scene
{
private:
	std::string name;
	std::map<Object*, std::vector<trans::Transformation*>> objects;
	std::vector<trans::Transformation*> transformations;

	Object* parseObject(const aiScene* scene, aiString path);

public:
	Scene(std::string name);
	bool load();

	std::map<Object*, std::vector<trans::Transformation*>> getObjects();

};

#endif