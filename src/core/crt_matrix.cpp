#include "crt_matrix.h"

#include <cmath>

namespace crt {

Matrix Matrix::rotation_x(const float angle_radians) {
    return {{
        {1.0f, 0.0f, 0.0f},
        {0.0f, std::cos(angle_radians), std::sin(angle_radians)},
        {0.0f, -std::sin(angle_radians), std::cos(angle_radians)}
    }};
}

Matrix Matrix::rotation_y(const float angle_radians) {
    return {{
        {std::cos(angle_radians), 0.0f, -std::sin(angle_radians)},
        {0.0f, 1.0f, 0.0f},
        {std::sin(angle_radians), 0.0f, std::cos(angle_radians)}
    }};
}

Matrix Matrix::rotation_z(const float angle_radians) {
    return {{
        {std::cos(angle_radians), std::sin(angle_radians), 0.0f},
        {-std::sin(angle_radians), std::cos(angle_radians), 0.0f},
        {0.0f, 0.0f, 1.0f}
    }};
}

}