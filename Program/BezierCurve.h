#pragma once
#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include <vector>
#include <glm/glm.hpp>

namespace trans {
	class Transformation;
	class Position;
}

class BezierCurve {
private:
	std::vector<glm::vec3> points;
	int order;
	float speed;
	
	trans::Position* position;
	trans::Transformation* transformation;
	int current_segment;
	float progress;
public:
	BezierCurve(std::vector<glm::vec3> points, int order, float speed);
	trans::Transformation* getTransformation();

	void move(double delta_time);
};

#endif
