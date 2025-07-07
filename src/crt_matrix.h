#pragma once

#include "crt_vector.h"

namespace crt {

struct Matrix {
    /**
     * Row-major 3x3 matrix.
     */
    float data[3][3]{};

    constexpr static Matrix identity() {
        Matrix result{};
        for (int i = 0; i < 3; i++) {
            result.data[i][i] = 1.0f;
        }
        return result;
    }

    static Matrix rotation_x(const float angle_radians);
    static Matrix rotation_y(const float angle_radians);
    static Matrix rotation_z(const float angle_radians);

    constexpr Matrix operator*(const Matrix &rhs) const {
        Matrix result{ *this };
        result *= rhs;
        return result;
    }

    constexpr Matrix operator*(const float scalar) const {
        Matrix result{ *this };
        result *= scalar;
        return result;
    }

    constexpr Matrix& operator*=(const Matrix &rhs) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                for (int k = 0; k < 3; k++) {
                    data[i][j] += data[i][k] * rhs.data[k][j];
                }
            }
        }
        return *this;
    }

    constexpr Matrix& operator*=(const float scalar) {
        *this = *this * scalar;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                data[i][j] = data[i][j] * scalar;
            }
        }
        return *this;
    }

    constexpr friend Vector operator*(const Vector &lhs_vec, const Matrix &rhs_mat) {
        Vector result{};
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.data[i] += lhs_vec.data[j] * rhs_mat.data[j][i];
            }
        }
        return result;
    }

    constexpr friend Vector &operator*=(Vector &lhs_vec, const Matrix &rhs_mat) {
        lhs_vec = lhs_vec * rhs_mat;
        return lhs_vec;
    }
};

}