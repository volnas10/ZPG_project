#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "Observers.h"

class Camera : public WindowSizeSubscriber {
private:
	glm::vec3 position;
	float fov;
	float horizontal_angle;
	float vertical_angle;
	float aspect_ratio;

	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;

	std::vector<CameraSubscriber*> matrix_subscribers;
public:
	Camera(glm::vec3 position, float fov, float horizontal_angle, float vertical_angle, float aspect_ratio);

	void subscribe(CameraSubscriber* subscriber);
	void notifySubscribers();

	void move(glm::vec3 dir, float h_angle, float v_angle);
	void changeFov(float value);
	void updateSize(int width, int height);

	glm::vec3 transformToWorldspace(glm::vec3 pos, glm::vec4 viewport);

	glm::mat4 getView();
	glm::mat4 getProjection();
	
};

#endif
