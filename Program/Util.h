#pragma once
#ifndef UTIL_H
#define UTIL_H

namespace transformation {
	class Transformation {
	private:
		// Nested classes
		class TransformationComponent {
		private:
			float x, y, z;
		public:
			TransformationComponent(float x, float y, float z);
		};

		class Position : TransformationComponent {
		public:
			Position(float x, float y, float z) : TransformationComponent(x, y, z) {};
		};

		class Rotation : TransformationComponent {
		public:
			Rotation(float x, float y, float z) : TransformationComponent(x, y, z) {};
		};

		class Scale : TransformationComponent {
		public:
			Scale(float x, float y, float z) : TransformationComponent(x, y, z) {};
		};

		std::vector<TransformationComponent> transformations;
	public:
		glm::mat4 getTransformation() {
		}
	};
}


#endif UTIL_H

