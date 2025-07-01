#pragma once

#include "crt_vector.h"
#include "crt_matrix.h"

namespace crt {

struct Transform {
    Vector location{};
    Matrix rotation{ Matrix::identity() };

    constexpr void translate_world(const Vector &translation) {
        location += translation;
    }

    constexpr Transform translated_world(const Vector &translation) const {
        return { location + translation, rotation };
    }

    constexpr void translate_local(const Vector &translation) {
        location += translation * rotation;
    }

    constexpr Transform translated_local(const Vector &translation) const {
        return { location + translation * rotation, rotation };
    }

    void rotate_x(const float angle_radians) {
        rotation *= Matrix::rotation_x(angle_radians);
    }

    void rotate_y(const float angle_radians) {
        rotation *= Matrix::rotation_y(angle_radians);
    }

    void rotate_z(const float angle_radians) {
        rotation *= Matrix::rotation_z(angle_radians);
    }

    Transform rotated_x(const float angle_radians) {
        return { location, rotation * Matrix::rotation_x(angle_radians) };
    }

    Transform rotated_y(const float angle_radians) {
        return { location, rotation * Matrix::rotation_y(angle_radians) };
    }

    Transform rotated_z(const float angle_radians) {
        return { location, rotation * Matrix::rotation_z(angle_radians) };
    }

};

}