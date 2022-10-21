#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <assimp/scene.h>
#include "Renderer.h"
#include "Util.h"

class Scene
{
private:
	std::string name;
	std::vector<AbstractRenderer*> renderers;
	std::vector<Program*> programs;
	std::vector<trans::Transformation*> transformations;
	trans::TransformationController transformation_controller;

	Object* parseObject(const aiScene* scene, aiString path);

public:
	Scene(std::string name);
	bool load();

	std::vector<AbstractRenderer*> getRenderers();
	std::vector<Program*> getPrograms();

	void moveObjects(double delta_time);

};

#endif