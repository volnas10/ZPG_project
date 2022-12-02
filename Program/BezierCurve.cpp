#include "Util.h"

#include "BezierCurve.h"

BezierCurve::BezierCurve(std::vector<glm::vec3> points, int order, float speed) {
	this->points = points;
	this->order = order;
	this->speed = speed;
	this->current_segment = 0;
	this->progress = 0;
	this->transformation = new trans::Transformation();
	this->position = transformation->translate(0, 0, 0);
}

trans::Transformation* BezierCurve::getTransformation() {
	return transformation;
}

void BezierCurve::move(double delta_time) {
	progress += (speed / order) * delta_time;
	if (progress > 1) {
		progress -= 1;
		int mod = (points.size() - ((points.size() - 1) % order)) - 1;
		current_segment = current_segment + order;
		if (current_segment >= mod) current_segment = 0;
	}

	glm::vec3 point_a, point_b;

	if (order > 1) {
		glm::vec3 point_x = points[current_segment];
		glm::vec3 point_y = points[current_segment + 1];
		glm::vec3 point_z = points[current_segment + 2];

		// For order 3 (cubic curve) reduce it to order 2 (quadratic curve)
		if (order == 3) {
			glm::vec3 vec_a = (points[current_segment + 1] - points[current_segment]) * progress;
			glm::vec3 vec_b = (points[current_segment + 2] - points[current_segment + 1]) * progress;
			glm::vec3 vec_c = (points[current_segment + 3] - points[current_segment + 2]) * progress;
			point_x += vec_a;
			point_y += vec_b;
			point_z += vec_c;
		}

		// Now reduce order 2 to order 1
		glm::vec3 vec_a = (point_y - point_x) * progress;
		glm::vec3 vec_b = (point_z - point_y) * progress;
		point_a = point_x + vec_a;
		point_b = point_y + vec_b;
	}
	else {
		point_a = points[current_segment];
		point_b = points[current_segment + 1];
	}

	// Calculate order 1 (linear) movement
	glm::vec3 vec = (point_b - point_a) * progress;
	vec = point_a + vec;
	position->set(vec.x, vec.y, vec.z);
}
