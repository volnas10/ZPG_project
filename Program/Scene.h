#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <map>
#include "Object.h"
#include "Util.h"

class Scene
{
private:
	std::string name;
	std::map<Object*, std::vector<trans::Transformation*>> objects;


public:
	Scene(std::string name);
	bool load();
	void save();
};

#endif