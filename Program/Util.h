#pragma once
#ifndef UTIL_H
#define UTIL_H

namespace transformation {
	class Transformation {
	private:
		// Nested classes
		class TransformationComponent {
		public:
			Transformation* parent;
		protected:
			float x, y, z;
		public:
			TransformationComponent(float x, float y, float z);
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

		glm::mat4 getTransformation();

		void changed();

		void operator<<(Transformation& b);

		static class Position : TransformationComponent {
		public:
			Position(float x, float y, float z) : TransformationComponent(x, y, z) {};
			glm::mat4 applyTransformation(glm::mat4 mat);
		};

		static class Rotation : TransformationComponent {
		public:
			Rotation(float x, float y, float z) : TransformationComponent(x, y, z) {};
			glm::mat4 applyTransformation(glm::mat4 mat);
		};

		static class Scale : TransformationComponent {
		public:
			Scale(float x, float y, float z) : TransformationComponent(x, y, z) {};
			glm::mat4 applyTransformation(glm::mat4 mat);
		};
	};
}


#endif UTIL_H

