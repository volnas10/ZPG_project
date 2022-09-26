#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
private:
	glm::vec3 position;
	float fov;
	float horizontal_angle;
	float vertical_angle;
	float aspect_ratio;

	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;
public:
	Camera(glm::vec3 position, float fov, float horizontal_angle, float vertical_angle, float aspect_ratio);

	void move(glm::vec3 dir, float h_angle, float v_angle);


	glm::mat4 getView();
	glm::mat4 getProjection();
	
};

#endif
