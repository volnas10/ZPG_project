#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"

namespace trans {
    Transformation::TransformationComponent::TransformationComponent(Transformation* parent, float x, float y, float z) {
        this->parent = parent;
        this->x = x;
        this->y = y;
        this->z = z;
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

    Transformation::~Transformation() {
        for (TransformationComponent* tc : transformations) {
            delete tc;
        }
    }

    glm::mat4 Transformation::getTransformation() {
        // If nothing changed, return saved transformation
        if (up_to_date) {
            return final_transformation;
        }

        // Else recalculate a new one
        final_transformation = glm::mat4(1.0);

        for (int i = transformations.size() - 1; i >= 0; i--) {
            final_transformation = transformations[i]->applyTransformation(final_transformation);
        }

        if (previous_transformation != nullptr) {
            final_transformation *= previous_transformation->getTransformation();
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

    void Transformation::operator<<(Transformation& trans) {
        previous_transformation = &trans;
        trans.subscribers.push_back(this);
        up_to_date = false;
    }

    void Transformation::operator<<(std::nullptr_t) {
        previous_transformation = nullptr;
        up_to_date = false;
        for (Transformation* sub : subscribers) {
            sub->changed();
        }
    }

    Transformation::Position* Transformation::translate(float x, float y, float z) {
        Position* pos = new Position(this, x, y, z);
        transformations.push_back(pos);
        up_to_date = false;
        return pos;
    }

    Transformation::Rotation* Transformation::rotate(float x, float y, float z) {
        Rotation* rotation = new Rotation(this, x, y, z);
        transformations.push_back(rotation);
        up_to_date = false;
        return rotation;
    }

    Transformation::Scale* Transformation::scale(float x, float y, float z) {
        Scale* scale = new Scale(this, x, y, z);
        transformations.push_back(scale);
        up_to_date = false;
        return scale;
    }

    Transformation::Scale* Transformation::scale(float size) {
        return scale(size, size, size);
    }


}
