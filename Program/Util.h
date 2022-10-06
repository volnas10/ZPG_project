#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <glm/gtc/matrix_transform.hpp>

namespace trans {
	class Transformation {
	private:
		// Nested classes
		class TransformationComponent {
		public:
			Transformation* parent;
		protected:
			float x, y, z;
		public:
			TransformationComponent(Transformation* parent, float x, float y, float z);
			void set(float x, float y, float z);
			virtual glm::mat4 applyTransformation(glm::mat4) = 0;
		};

		// Vector of transformations that depend on this transformation
		std::vector<Transformation*> subscribers;
		Transformation* previous_transformation;

		std::vector<TransformationComponent*> transformations;

		bool up_to_date;
		glm::mat4 final_transformation;
	public:
		Transformation();
		Transformation(Transformation* trans);
		~Transformation();

		glm::mat4 getTransformation();

		void changed();

		void operator<<(Transformation& b);
		void operator<<(std::nullptr_t);

		class Position : public TransformationComponent {
		public:
			Position(Transformation* parent, float x, float y, float z) : TransformationComponent(parent, x, y, z) {};
			glm::mat4 applyTransformation(glm::mat4 mat);
		};

		class Rotation : public TransformationComponent {
		public:
			Rotation(Transformation* parent, float x, float y, float z) : TransformationComponent(parent, x, y, z) {};
			glm::mat4 applyTransformation(glm::mat4 mat);
		};

		class Scale : public TransformationComponent {
		public:
			Scale(Transformation* parent, float x, float y, float z) : TransformationComponent(parent, x, y, z) {};
			glm::mat4 applyTransformation(glm::mat4 mat);
		};

		Position* translate(float x, float y, float z);
		Rotation* rotate(float x, float y, float z);
		Scale* scale(float x, float y, float z);
		Scale* scale(float size);
	};
}


#endif UTIL_H

