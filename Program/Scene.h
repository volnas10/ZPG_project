#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <assimp/scene.h>
#include "Renderer.h"
#include "Camera.h"
#include "Util.h"

class Scene
{
private:
	std::string name;
	std::vector<AbstractRenderer*> renderers;
	std::vector<Program*> programs;
	std::vector<trans::Transformation*> transformations;
	trans::TransformationController transformation_controller;
	Camera* camera;

	object::Object* parseObject(const aiScene* scene, aiString path);

public:
	Scene(std::string name);
	~Scene();
	bool load();

	std::vector<AbstractRenderer*> getRenderers();
	std::vector<Program*> getPrograms();
	Camera* getCamera();

	void moveObjects(double delta_time);

};

#endif