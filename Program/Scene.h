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
	std::vector<AbstractRenderer*> pre_renderers;
	std::vector<Renderer*> main_renderers;
	std::vector<AbstractRenderer*> post_renderers;

	std::vector<object::Object*> models;
	object::ObjectGroup* objects;
	std::vector<trans::Transformation*> transformations;
	trans::TransformationController transformation_controller;
	LightCollection* lights;
	Camera* camera;
	std::vector<WindowSizeSubscriber*> window_subscribers;

	unsigned int shadow_type;

	int findAdjacencedIndex(const aiMesh* mesh, int vertex1, int vertex2, int vertex3);
	object::Object* parseObject(const aiScene* scene, aiString path);
	std::vector<float> parseVertices(const aiScene* scene);

public:
	Scene(std::string name);
	~Scene();
	bool load();

	std::vector<object::Object*> getObjects();
	object::ObjectGroup* getObjectGroup();
	RENDERERS getRenderers();
	std::vector<WindowSizeSubscriber*> getWindowSubscribers();
	Camera* getCamera();
	LightCollection* getLights();
	unsigned int getShadowType();

	void moveObjects(double delta_time);

};

#undef VAR
#endif