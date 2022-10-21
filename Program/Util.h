#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace trans {

	class TransformationComponent {
	protected:
		glm::mat4 calculated_transformation;
		bool up_to_date;
	public:
		virtual glm::mat4 getTransformation() = 0;
	};

	class Transformation;

	class TransformationLeaf : public TransformationComponent {
	protected:
		Transformation* parent;
		float x, y, z;
	public:
		TransformationLeaf(Transformation* parent, float x, float y, float z);
		void set(float x, float y, float z);
		void add(float x, float y, float z);
	};

	class Position : public TransformationLeaf {
	public:
		Position(Transformation* parent, float x, float y, float z) : TransformationLeaf(parent, x, y ,z) {};
		glm::mat4 getTransformation();
	};

	class Rotation : public TransformationLeaf {
	public:
		Rotation(Transformation* parent, float x, float y, float z) : TransformationLeaf(parent, x, y, z) {};
		glm::mat4 getTransformation();
	};

	class Scale : public TransformationLeaf {
	public:
		Scale(Transformation* parent, float x, float y, float z) : TransformationLeaf(parent, x, y, z) {};
		glm::mat4 getTransformation();
	};

	class Transformation : TransformationComponent {
	private:
		std::vector<Transformation*> parents;
		// Vector of transformations that depend on this transformation
		std::vector<TransformationComponent*> transformations;
	public:
		Transformation();

		glm::mat4 getTransformation();

		void changed();

		void operator<<(Transformation& b);
		void addParent(Transformation* t);

		Position* translate(float x, float y, float z);
		Rotation* rotate(float x, float y, float z);
		Scale* scale(float x, float y, float z);
		Scale* scale(float size);
	};

	class TransformationController {
	private:
		std::unordered_map<TransformationLeaf*, glm::vec3> changes;
	public:
		void addChange(TransformationLeaf* t, glm::vec3 change);
		void move(double delta_time);
	};
}


#endif

