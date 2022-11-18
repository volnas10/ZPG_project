#define GLEW_STATIC
#include <GL/glew.h>
#include <map>
#include <vector>
#include <iostream>

#include "TransformationBuffer.h"
#include "Renderer.h"
#include "ShadowMapper.h"

#include "RenderingScheduler.h"


RenderingScheduler::RenderingScheduler() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	use_shadows = false;
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

void RenderingScheduler::setLights(LightCollection* lights) {
	this->lights = lights;
	lights->notifySubscribers();
}

void RenderingScheduler::render() {
	// Instead of going through all objects in renderer we go through all instances of the same mesh in each renderer
	// So instead of preparing one mesh again andagain for each instance, we prepare it once and draw every instance in every renderer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (use_shadows) {
		for (MeshInstances meshInstances : meshes) {
			meshInstances.mesh->bind();
			shadow_mapper->renderShadows(meshInstances.mesh);
		}
	}


	for (AbstractRenderer* r : other_renderers) {
		r->render();
	}
	for (MeshInstances meshInstances : meshes) {
		meshInstances.mesh->bind();
		for (auto pair : meshInstances.instances) {
			// Use renderer
			int transformations_idx;
			pair.first->prepare(&transformations_idx);
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
