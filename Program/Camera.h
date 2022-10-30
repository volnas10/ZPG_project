#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "Program.h"

class Camera {
private:
	glm::vec3 position;
	float fov;
	float horizontal_angle;
	float vertical_angle;
	float aspect_ratio;

	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;

	std::vector<Program*> subscribers;
public:
	Camera(glm::vec3 position, float fov, float horizontal_angle, float vertical_angle, float aspect_ratio);

	void subscribe(Program* program);
	void notifySubscribers();

	void move(glm::vec3 dir, float h_angle, float v_angle);
	void changeFov(double value);
	void changeAspectRatio(float aspect_ratio);

	glm::mat4 getView();
	glm::mat4 getProjection();
	
};

#endif
