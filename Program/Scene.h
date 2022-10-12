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

	void parseObject(const aiScene* scene);

public:
	Scene(std::string name);
	bool load();
	void save();

	std::map<Object*, std::vector<trans::Transformation*>> getObjects();

};

#endif