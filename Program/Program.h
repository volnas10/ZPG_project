#pragma once
#ifndef PROGRAM_H
#define PROGRAM_H

#define GLEW_STATIC
#include <GL/glew.h>

#include "Light.h"
#include "Observers.h"
#include "Shader.h"

#include <vector>

class Program : public CameraSubscriber, public LightSubscriber {
private:
	GLuint program_ID;

	GLuint view_ID, projection_view_ID;
	GLuint lights_buffer, light_count_ID;
public:
	Program();
	Program(std::vector<Shader> shaders);
	~Program();

	void use();
	void stopUsing();

	void updateCamera(glm::mat4 view_matrix, glm::mat4 projection_matrix);
	void updateLights(std::vector<Light*> lights);

	GLuint getUniformLocation(std::string name);
	GLuint getUniformBlockLocation(std::string name, int pos);
};

#endif
