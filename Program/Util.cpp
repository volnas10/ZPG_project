#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "TransformationBuffer.h"

#include "Util.h"

namespace trans {
	TransformationLeaf::TransformationLeaf(Transformation* parent, float x, float y, float z) {
		this->parent = parent;
		this->x = x;
		this->y = y;
		this->z = z;
		up_to_date = false;
	}
	void TransformationLeaf::set(float x, float y, float z) {
		parent->changed();
		up_to_date = false;
		this->x = x;
		this->y = y;
		this->z = z;
	}
	void TransformationLeaf::add(float x, float y, float z) {
		parent->changed();
		up_to_date = false;
		this->x += x;
		this->y += y;
		this->z += z;
	}

	glm::mat4 Position::getTransformation() {
		if (!up_to_date) {
			calculated_transformation = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));
			up_to_date = true;
		}
		return calculated_transformation;
	}

	glm::mat4 Rotation::getTransformation() {
		if (!up_to_date) {
			glm::mat4 mat(1.0);
			glm::vec3 axis(x != 0, y != 0, z != 0);
			float angle = x != 0 ? x : y != 0 ? y : z;
			calculated_transformation = glm::rotate(glm::mat4(1.0), angle, axis);
			up_to_date = true;
		}
		return calculated_transformation;
	}

	glm::mat4 Scale::getTransformation() {
		if (!up_to_date) {
			calculated_transformation = glm::scale(glm::mat4(1.0), glm::vec3(x, y, z));
			up_to_date = true;
		}
		return calculated_transformation;
	}
	Transformation::Transformation() {
		calculated_transformation = glm::mat4(1.0);
		up_to_date = false;
	}
	glm::mat4 Transformation::getTransformation() {
		if (!up_to_date) {
			calculated_transformation = glm::mat4(1.0);
			for (TransformationComponent* t : transformations) {
				calculated_transformation = t->getTransformation() * calculated_transformation;
			}
			up_to_date = true;
		}
		return calculated_transformation;
	}

	void Transformation::changed() {
		up_to_date = false;
		for (Transformation* t : parents) {
			t->changed();
		}
		for (auto pair : dependent_buffers) {
			pair.first->updateTransformation(pair.second, getTransformation());
		}
	}

	void Transformation::operator<<(Transformation& b) {
		transformations.push_back(&b);
		b.addParent(this);
	}

	void Transformation::addParent(Transformation* t) {
		parents.push_back(t);
		up_to_date = false;
	}

	void Transformation::addDependency(size_t index, TransformationBuffer* buffer) {
		dependent_buffers.push_back(std::make_pair(buffer, index));
	}

	Position* Transformation::translate(float x, float y, float z) {
		Position* pos = new Position(this, x, y, z);
		transformations.push_back(pos);
		up_to_date = false;
		return pos;
	}

	Rotation* Transformation::rotate(float x, float y, float z) {
		Rotation* rot = new Rotation(this, x, y, z);
		transformations.push_back(rot);
		up_to_date = false;
		return rot;
	}

	Scale* Transformation::scale(float x, float y, float z) {
		Scale* s = new Scale(this, x, y, z);
		transformations.push_back(s);
		up_to_date = false;
		return s;
	}

	Scale* Transformation::scale(float size) {
		return scale(size, size, size);
	}

	void Transformation::addMatrix(glm::mat4 mat) {
		transformations.push_back(new MatTrans(mat));
		up_to_date = false;
	}


	void TransformationController::addChange(TransformationLeaf* t, glm::vec3 change) {
		changes[t] = change;
	}

	void TransformationController::move(double delta_time) {
		for (auto pair : changes) {
			pair.first->add(
				(float) pair.second.x * delta_time,
				(float) pair.second.y * delta_time,
				(float) pair.second.z * delta_time);
		}
	}

	MatTrans::MatTrans(glm::mat4 mat) {
		calculated_transformation = mat;
		up_to_date = true;
	}

	glm::mat4 MatTrans::getTransformation() {
		return calculated_transformation;
	}

	TransformationGenerator::TransformationGenerator() {
		min_distance = 0;
	}

	void TransformationGenerator::setMinDistance(float distance) {
		min_distance = distance;
	}

	void TransformationGenerator::addObstacles(std::vector<glm::vec3> obstacles) {
		for (glm::vec3 obstacle : obstacles) {
			this->obstacles.push_back(obstacle);
		}
	}

	std::vector<glm::vec3> TransformationGenerator::getObstacles() {
		return obstacles;
	}

	std::vector<Transformation*> TransformationGenerator::generateTransformations(int count, glm::vec3 bound1, glm::vec3 bound2, Transformation* previous) {
		std::random_device rd;
		std::mt19937 e2(rd());

		std::uniform_real_distribution<> x_generator(bound1.x, bound2.x);
		std::uniform_real_distribution<> y_generator(bound1.y, bound2.y);
		std::uniform_real_distribution<> z_generator(bound1.z, bound2.z);

		int i = 0;
		int generated = 0;
		while (i < 10000000) {
			float x = x_generator(e2);
			float y = y_generator(e2);
			float z = z_generator(e2);

			// Check if minimum distance is met
			if (min_distance > 0) {
				for (glm::vec3 obstacle : obstacles) {
					float dist = 0;
					if (bound1.x != bound2.x) {
						dist += std::pow(x - obstacle.x, 2);
					}
					if (bound1.y != bound2.y) {
						dist += std::pow(y - obstacle.y, 2);
					}
					if (bound1.z != bound2.z) {
						dist += std::pow(z - obstacle.z, 2);
					}

					if (std::sqrt(dist) < min_distance) {
						continue;
					}
				}
			}
			obstacles.push_back(glm::vec3(x, y, z));
			generated++;
			if (generated == count) {
				break;
			}

			i++;
		}

		if (i == 10000000) {
			std::cout << "Failed to generate desired amount of transformations" << std::endl;
		}

		std::uniform_real_distribution<> size_generator(0.7, 1.2);
		std::uniform_real_distribution<> rotation_generator(0, 3.14);

		std::vector<Transformation*> transformations;
		for (int j = obstacles.size() - generated; j < obstacles.size(); j++) {
			Transformation* t = new Transformation();
			if (previous != nullptr) {
				*t << *previous;
			}
			t->scale(size_generator(e2));
			// Just rotate around Y randomly... Not like this is going to be used for any other scenario
			t->rotate(0, rotation_generator(e2), 0);
			t->translate(obstacles[j].x, obstacles[j].y, obstacles[j].z);
			transformations.push_back(t);
		}

		return transformations;
	}

}

namespace stringutil {
	void stringutil::replaceChar(char* ptr, char from, char to) {
		int i = 0;
		while (ptr[i] != 0) {
			if (ptr[i] == from) {
				ptr[i] = to;
			}
			i++;
		}
	}
}


