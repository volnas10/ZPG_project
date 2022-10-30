#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "Util.h"
#include "Camera.h"

Camera::Camera(glm::vec3 position, float fov, float horizontal_angle, float vertical_angle, float aspect_ratio) {
	this->position = position;
	this->fov = fov;
	this->horizontal_angle = horizontal_angle;
	this->vertical_angle = vertical_angle;
	this->aspect_ratio = aspect_ratio;

	// Get view and projection matrix calculated right away
	projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 150.0f);
	move(glm::vec3(0), 0, 0);
}

void Camera::subscribe(Program* program) {
	subscribers.push_back(program);
	program->notify(view_matrix, projection_matrix);
}

void Camera::notifySubscribers() {
	for (Program* program : subscribers) {
		program->notify(view_matrix, projection_matrix);
	}
}

void Camera::move(glm::vec3 dir, float h_angle, float v_angle) {
	horizontal_angle = std::fmod(horizontal_angle + h_angle, 2 * PI);
	vertical_angle = std::fmod(vertical_angle + v_angle, 2 * PI);

	if (vertical_angle < -PI / 2) vertical_angle = -PI / 2;
	else if (vertical_angle > PI / 2) vertical_angle = PI / 2;

	glm::vec3 direction(
	    cos(vertical_angle) * sin(horizontal_angle),
	    sin(vertical_angle),
	    cos(vertical_angle) * cos(horizontal_angle)
	);

	glm::vec3 right(
	    sin(horizontal_angle - glm::pi<float>() / 2.0f),
	    0,
	    cos(horizontal_angle - glm::pi<float>() / 2.0f)
	);

	glm::vec3 up = glm::cross(right, direction);

	position += direction * dir.z;
	position += right * dir.x;
	position.y += dir.y;

	view_matrix = glm::lookAt(
	    position,
	    position + direction,
	    up
	);

	notifySubscribers();
}

void Camera::changeFov(double value) {
	float new_fov = fov - value;
	if (new_fov > 20 && new_fov < 120) {
		fov = new_fov;
	}
	projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 150.0f);
	notifySubscribers();
}

void Camera::changeAspectRatio(float aspect_ratio) {
	this->aspect_ratio = aspect_ratio;
	projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 150.0f);
	notifySubscribers();
}

glm::mat4 Camera::getView() {
	return view_matrix;
}

glm::mat4 Camera::getProjection() {
	return projection_matrix;
}
