#include "crt_vector.h"

#include <cmath>

namespace crt {

float Vector::length() const {
    return std::sqrt(length_squared());
}

bool Vector::refract(Vector normal, float outside_ior, float inside_ior) {
    float cos_alpha = -this->dot(normal);
    float sin_alpha = std::sqrt(1.0f - cos_alpha * cos_alpha);

    if (sin_alpha > inside_ior / outside_ior)
        return false;

    float sin_beta = sin_alpha * outside_ior / inside_ior;
    float cos_beta = std::sqrt(1.0f - sin_beta * sin_beta);

    *this += normal * cos_alpha;
    normalize();
    *this *= sin_beta;
    *this += -normal * cos_beta;

    return true;
}

}