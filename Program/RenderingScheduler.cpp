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
	use_shadows = false;

	//depth_map_renderer = new DepthMapRenderer();
}

void RenderingScheduler::addOtherRenderers(std::vector<AbstractRenderer*> renderers) {
	for (AbstractRenderer* r : renderers) {
		other_renderers.push_back(r);
	}
}

// Parses all objects into a structure that is more efficient to render
void RenderingScheduler::addRenderingGroups(std::vector<object::Object*> objects, std::vector<RenderingGroup*> groups) {
	for (object::Object* object : objects) {
		for (RenderingGroup* group : groups) {
			std::vector<trans::Transformation*> transformations = group->getTransformations(object);
			if (transformations.size() == 0) {
				continue;
			}

			for (object::Mesh* mesh : object->getMeshes()) {
				TransformationBuffer* buffer = new TransformationBuffer();
				buffer->setTransformations(transformations);
				MeshInstances mi;
				mi.mesh = mesh;
				mi.instances[group->getRenderer()] = buffer;
				meshes.push_back(mi);
			}
		}
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
					pair.second->bind(transformations_idx);

					shadow_mapper->renderShadows(meshInstances.mesh, pair.second->size());
				}
			}

		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, viewport_width, viewport_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);

	//depth_map_renderer->render(shadow_mapper->getUnit());

	for (AbstractRenderer* r : other_renderers) {
		r->render();
	}
	for (MeshInstances meshInstances : meshes) {
		meshInstances.mesh->bind();
		for (auto pair : meshInstances.instances) {
			// Use renderer
			int transformations_idx;
			pair.first->prepare(&transformations_idx, shadow_mapper->getUnit());
			pair.second->bind(transformations_idx);

			// Draw the mesh with every transformation
			pair.first->render(meshInstances.mesh, pair.second->size());
		}
	}

}

void RenderingScheduler::useShadows() {
	use_shadows = true;
	shadow_mapper = new ShadowMapper();
}

ShadowMapper* RenderingScheduler::getShadowMapper() {
	return shadow_mapper;
}
