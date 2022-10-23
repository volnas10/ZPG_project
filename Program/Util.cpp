#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
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
	}

	void Transformation::operator<<(Transformation& b) {
		transformations.push_back(&b);
		b.addParent(this);
	}

	void Transformation::addParent(Transformation* t) {
		parents.push_back(t);
		up_to_date = false;
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

}
