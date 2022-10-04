#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Util.h"

namespace transformation {
    Transformation::TransformationComponent::TransformationComponent(float x, float y, float z) {
        this->set(x, y, z);
    }
    void Transformation::TransformationComponent::set(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
        parent->changed();
    }
    glm::mat4 Transformation::Position::applyTransformation(glm::mat4 mat) {
        return glm::translate(mat, glm::vec3(x, y, z));
    }
    glm::mat4 Transformation::Rotation::applyTransformation(glm::mat4 mat) {
        if (x != 0) {
            mat = glm::rotate(mat, x, glm::vec3(1, 0, 0));
        }
        if (y != 0) {
            mat = glm::rotate(mat, y, glm::vec3(0, 1, 0));
        }
        if (z != 0) {
            mat = glm::rotate(mat, z, glm::vec3(0, 0, 1));
        }

        return mat;
    }
    glm::mat4 Transformation::Scale::applyTransformation(glm::mat4 mat) {
        return glm::scale(mat, glm::vec3(x, y, z));
    }
    Transformation::Transformation() {
        previous_transformation = nullptr;
        final_transformation = glm::mat4(1.0);
        up_to_date = true;
    }
    Transformation::Transformation(Transformation* trans) {
        previous_transformation = trans;
        final_transformation = previous_transformation->getTransformation();
        up_to_date = true;
    }
    glm::mat4 Transformation::getTransformation() {
        // If nothing changed, return saved transformation
        if (up_to_date) {
            return final_transformation;
        }

        // Else recalculate a new one
        final_transformation = previous_transformation->getTransformation();
        for (TransformationComponent* tc : transformations) {
            final_transformation = tc->applyTransformation(final_transformation);
        }
        up_to_date = true;

        return final_transformation;
    }
    void Transformation::changed() {
        up_to_date = false;
        for (Transformation* sub : subscribers) {
            sub->changed();
        }
    }
    void Transformation::operator<<(Transformation& b) {
        previous_transformation = &b;
        b.subscribers.push_back(this);
        up_to_date = false;
    }
}
