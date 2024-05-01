#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "Util.h"
#include "Camera.h"

void Camera::calcProjectionMatrix() {
	float near = 0.1f;
	float far = 150.0f;

	float a = (near + far) / (near - far);
	float b = (2 * near * far) / (near - far);

	float y = 1 / tanf(glm::radians(fov) / 2);
	float x = y / aspect_ratio;
	projection_matrix = glm::mat4();
	projection_matrix[0] = glm::vec4(x, 0, 0, 0);
	projection_matrix[1] = glm::vec4(0, y, 0, 0);
	projection_matrix[2] = glm::vec4(0, 0, a, -1);
	projection_matrix[3] = glm::vec4(0, 0, b, 0);
}

Camera::Camera(glm::vec3 position, float fov, float horizontal_angle, float vertical_angle) {
	this->position = position;
	this->fov = fov;
	this->horizontal_angle = horizontal_angle;
	this->vertical_angle = vertical_angle;
	this->view_matrix = glm::mat4();
	this->projection_matrix = glm::mat4();

	move(glm::vec3(0), 0, 0);
}

void Camera::subscribe(CameraSubscriber* subscriber) {
	matrix_subscribers.push_back(subscriber);
	subscriber->updateCamera(view_matrix, projection_matrix);
}

void Camera::notifySubscribers() {
	for (CameraSubscriber* sub : matrix_subscribers) {
		sub->updateCamera(view_matrix, projection_matrix);
	}
}

void Camera::move(glm::vec3 dir, float h_angle, float v_angle) {
	horizontal_angle = (float) std::fmodf(horizontal_angle + h_angle, 2 * PI);
	vertical_angle = (float) std::fmodf(vertical_angle + v_angle, 2 * PI);

	if (vertical_angle < -PI / 2) vertical_angle = (float)  - PI / 2;
	else if (vertical_angle > PI / 2) vertical_angle = (float) PI / 2;

	glm::vec3 direction(
	    cosf(vertical_angle) * sinf(horizontal_angle),
	    sinf(vertical_angle),
	    cosf(vertical_angle) * cosf(horizontal_angle)
	);


	glm::vec3 right(
	    sinf(horizontal_angle - glm::pi<float>() / 2.0f),
	    0,
	    cosf(horizontal_angle - glm::pi<float>() / 2.0f)
	);

	glm::vec3 up = glm::cross(right, direction);

	position += direction * dir.z;
	position += right * dir.x;
	position.y += dir.y;

	//view_matrix = glm::lookAt(
	//    position,
	//    position + direction,
	//    up
	//);
	view_matrix[0] = glm::vec4(right, 0);
	view_matrix[1] = glm::vec4(up, 0);
	view_matrix[2] = glm::vec4(-direction, 0);
	view_matrix[3] = glm::vec4(position, 1);
	view_matrix = glm::inverse(view_matrix);

	notifySubscribers();
}

void Camera::changeFov(float value) {
	float new_fov = fov - value;
	if (new_fov > 20 && new_fov < 120) {
		fov = new_fov;
	}
	//projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 150.0f);
	calcProjectionMatrix();
	notifySubscribers();
}

void Camera::updateSize(int width, int height)
{
	this->aspect_ratio = (float) width / height;
	//projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 150.0f);
	calcProjectionMatrix();
	notifySubscribers();
}

glm::vec3 Camera::transformToWorldspace(glm::vec3 pos, glm::vec4 viewport) {
	return glm::unProject(pos, view_matrix, projection_matrix, viewport);
}

glm::mat4 Camera::getView() {
	return view_matrix;
}

glm::mat4 Camera::getProjection() {
	return projection_matrix;
}
