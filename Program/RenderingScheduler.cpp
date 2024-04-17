#define GLEW_STATIC
#include <GL/glew.h>
#include <map>
#include <vector>
#include <iostream>

#include "TransformationBuffer.h"
#include "Renderer.h"
#include "Util.h"

#include "RenderingScheduler.h"


RenderingScheduler::RenderingScheduler(object::ObjectGroup* group, RENDERERS renderers) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenFramebuffers(1, &depth_FBO);
	shadow_type = SHADOWS_NONE;

	//depth_map_renderer = new DepthMapRenderer();

	pre_renderers = std::get<0>(renderers);
	main_renderers = std::get<1>(renderers);
	post_renderers = std::get<2>(renderers);

	for (auto obj_group : group->objects) {
		object::Object* object = obj_group.first;
		std::vector<trans::Transformation*> transformations = obj_group.second.second;
		trans::Transformation* default_trans = obj_group.second.first;

		if (transformations.size() == 0) {
			continue;
		}
		
		TransformationBuffer* buffer = new TransformationBuffer();
		buffer->setDefaultTransformation(default_trans);
		buffer->setTransformations(transformations);
		for (object::Mesh* mesh : object->getMeshes()) {
			MeshInstances mi;
			mi.mesh = mesh;
			mi.instances = buffer;
			meshes.push_back(mi);
		}
	}
}

void RenderingScheduler::updateLights(std::vector<Light::LightStruct> lights) {
	this->lights = lights;
}

void RenderingScheduler::render(float viewport_width, float viewport_height) {
	// Instead of going through all objects in renderer we go through all instances of the same mesh in each renderer
	// So instead of preparing one mesh again andagain for each instance, we prepare it once and draw every instance in every renderer

	if (shadow_type == SHADOWS_MAP) {
		int transformations_idx;
		shadow_mapper->prepare(&transformations_idx);

		for (unsigned int i = 0; i < lights.size(); i++) {

			shadow_mapper->useLight(lights[i], i);

			for (MeshInstances meshInstances : meshes) {
				meshInstances.mesh->bindForShadows();
				meshInstances.instances->bind(transformations_idx);
				shadow_mapper->renderShadows(meshInstances.mesh, meshInstances.instances->size());
			}

		}
	}
	else if (shadow_type == SHADOWS_STENCIL) {
		// Start with ambient pass
		for (MeshInstances meshInstances : meshes) {
			meshInstances.mesh->bind();
			int transformations_idx;
			((Renderer*)main_renderers[0])->prepare(&transformations_idx, -1);
			meshInstances.instances->bind(transformations_idx);
			((Renderer*)main_renderers[0])->render(meshInstances.mesh, meshInstances.instances->size());
		}

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_CLAMP);
		glDisable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);


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

		// Use renderer
		int transformations_idx;
		if (shadow_type == SHADOWS_MAP) {
			((Renderer*) main_renderers[0])->prepare(&transformations_idx, shadow_mapper->getUnit());
		}
		else {
			((Renderer*)main_renderers[0])->prepare(&transformations_idx, -1);
		}
		meshInstances.instances->bind(transformations_idx);

		// Draw the mesh with every transformation
		((Renderer*)main_renderers[0])->render(meshInstances.mesh, meshInstances.instances->size());
	}

	glDisable(GL_STENCIL_TEST);


	for (AbstractRenderer* r : post_renderers) {
		r->render();
	}
}

void RenderingScheduler::setShadowType(unsigned int type) {
	shadow_type = type;
	if (type != SHADOWS_NONE) {
		shadow_mapper = new ShadowMapper();
	}
}

void RenderingScheduler::addObjectAtRuntime(trans::Transformation* transformation) {
	/*
	std::cout << "Adding object with id " << selected_object_id << std::endl;
	for (MeshInstances instances : meshes) {
		for (auto pair : instances.instances) {
			if (pair.first != selected_object_id) continue;
			pair.second.second->addTransformation(transformation);
		}
	}
	*/
}

float RenderingScheduler::depthAtPos(int width, int height) {
	float z;
	glReadPixels(width, height, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	return z;
}

ShadowMapper* RenderingScheduler::getShadowMapper() {
	return shadow_mapper;
}
