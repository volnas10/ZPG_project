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
	std::vector<RenderingGroup*> rendering_groups;
	std::vector<AbstractRenderer*> other_renderers;
	std::vector<Program*> programs;
	std::vector<object::Object*> models;
	std::vector<std::pair<object::Object*, std::vector<trans::Transformation*>>> objects;
	std::vector<trans::Transformation*> transformations;
	trans::TransformationController transformation_controller;
	LightCollection* lights;
	Camera* camera;

	object::Object* parseObject(const aiScene* scene, aiString path);

public:
	Scene(std::string name);
	~Scene();
	bool load();

	std::vector<object::Object*> getObjects();
	std::vector<RenderingGroup*> getRenderingGroups();
	std::vector<AbstractRenderer*> getRenderers();
	std::vector<Program*> getPrograms();
	Camera* getCamera();
	LightCollection* getLights();

	void moveObjects(double delta_time);

};

#endif