#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <random>

#define PI 3.14159

class TransformationBuffer;

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

	class MatTrans : public TransformationComponent {
	public:
		MatTrans(glm::mat4);
		glm::mat4 getTransformation();
	};

	class Transformation : TransformationComponent {
	private:
		std::vector<Transformation*> parents;

		// Vector of transformation buffers containing this transormation
		std::vector<std::pair<TransformationBuffer*, size_t>> dependent_buffers;

		// Vector of transformations that depend on this transformation
		std::vector<TransformationComponent*> transformations;
	public:
		Transformation();

		glm::mat4 getTransformation();

		void changed();

		void operator<<(Transformation& b);
		void addParent(Transformation* t);
		void addDependency(size_t index, TransformationBuffer* buffer);

		Position* translate(float x, float y, float z);
		Rotation* rotate(float x, float y, float z);
		Scale* scale(float x, float y, float z);
		Scale* scale(float size);
		void addMatrix(glm::mat4 mat);
	};

	class TransformationController {
	private:
		std::unordered_map<TransformationLeaf*, glm::vec3> changes;
	public:
		void addChange(TransformationLeaf* t, glm::vec3 change);
		void move(double delta_time);
	};

	// Class for generating random transformations for testing
	class TransformationGenerator {
	private:
		float min_distance;
		std::vector<glm::vec3> obstacles;
	public:
		TransformationGenerator();
		void setMinDistance(float distance);
		void addObstacles(std::vector<glm::vec3> obstacles);
		std::vector<glm::vec3> getObstacles();
		std::vector<Transformation*> generateTransformations(int count, glm::vec3 bound1, glm::vec3 bound2, Transformation* previous);
	};
}

namespace stringutil {
	void replaceChar(char* ptr, char from, char to);
}



#endif

