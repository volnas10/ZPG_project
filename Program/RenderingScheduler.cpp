#define GLEW_STATIC
#include <GL/glew.h>
#include <map>
#include <vector>
#include <iostream>

#include "TransformationBuffer.h"
#include "Renderer.h"

#include "RenderingScheduler.h"


RenderingScheduler::RenderingScheduler() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenFramebuffers(1, &depth_FBO);
	use_shadows = false;

	//depth_map_renderer = new DepthMapRenderer();
}

void RenderingScheduler::addPreRenderer(AbstractRenderer* renderer) {
	pre_renderers.push_back(renderer);
}

void RenderingScheduler::addPostRenderer(AbstractRenderer* renderer) {
	post_renderers.push_back(renderer);
}


// Parses all objects into a structure that is more efficient to render
void RenderingScheduler::addRenderingGroups(std::vector<object::Object*> objects, std::vector<RenderingGroup*> groups) {
	int object_id = 0;
	for (object::Object* object : objects) {
		for (RenderingGroup* group : groups) {
			std::vector<trans::Transformation*> transformations = group->getTransformations(object);
			trans::Transformation* default_trans = group->getDefaultTransformation(object);
			if (transformations.size() == 0) {
				continue;
			}

			TransformationBuffer* buffer = new TransformationBuffer();
			buffer->setDefaultTransformation(default_trans);
			buffer->setTransformations(transformations);
			for (object::Mesh* mesh : object->getMeshes()) {
				MeshInstances mi;
				mi.mesh = mesh;
				mi.instances[object_id + group->id] = std::make_pair(group->getRenderer(), buffer);
				meshes.push_back(mi);
			}
		}
		object_id += 10;
	}
}

void RenderingScheduler::updateLights(std::vector<Light::LightStruct> lights) {
	this->lights = lights;
}

void RenderingScheduler::render(float viewport_width, float viewport_height) {
	// Instead of going through all objects in renderer we go through all instances of the same mesh in each renderer
	// So instead of preparing one mesh again andagain for each instance, we prepare it once and draw every instance in every renderer

	if (use_shadows) {
		int transformations_idx;
		shadow_mapper->prepare(&transformations_idx);

		for (unsigned int i = 0; i < lights.size(); i++) {

			shadow_mapper->useLight(lights[i], i);

			for (MeshInstances meshInstances : meshes) {
				meshInstances.mesh->bindForShadows();
				for (auto pair : meshInstances.instances) {
					auto target = pair.second;
					target.second->bind(transformations_idx);

					shadow_mapper->renderShadows(meshInstances.mesh, target.second->size());
				}
			}

		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, viewport_width, viewport_height);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glCullFace(GL_BACK);

	//depth_map_renderer->render(shadow_mapper->getUnit());
	
	for (AbstractRenderer* r : pre_renderers) {
		r->render();
	}
	for (MeshInstances meshInstances : meshes) {
		meshInstances.mesh->bind();
		for (auto pair : meshInstances.instances) {
			glStencilFunc(GL_ALWAYS, pair.first, 0xFF);
			auto target = pair.second;
			// Use renderer
			int transformations_idx;
			if (use_shadows) {
				target.first->prepare(&transformations_idx, shadow_mapper->getUnit());
			}
			else {
				target.first->prepare(&transformations_idx, -1);
			}
			target.second->bind(transformations_idx);

			// Draw the mesh with every transformation
			target.first->render(meshInstances.mesh, target.second->size());
		}
	}

	glDisable(GL_STENCIL_TEST);


	for (AbstractRenderer* r : post_renderers) {
		r->render();
	}
	
}

void RenderingScheduler::useShadows() {
	use_shadows = true;
	shadow_mapper = new ShadowMapper();
}

void RenderingScheduler::addObjectAtRuntime(trans::Transformation* transformation) {
	std::cout << "Adding object with id " << selected_object_id << std::endl;
	for (MeshInstances instances : meshes) {
		for (auto pair : instances.instances) {
			if (pair.first != selected_object_id) continue;
			pair.second.second->addTransformation(transformation);
		}
	}
}

void RenderingScheduler::selectObject(int width, int height) {
	glReadPixels(width, height, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &selected_object_id);
	std::cout << "Selected object with id: " << selected_object_id << std::endl;
}

float RenderingScheduler::depthAtPos(int width, int height) {
	float z;
	glReadPixels(width, height, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	return z;
}

ShadowMapper* RenderingScheduler::getShadowMapper() {
	return shadow_mapper;
}
