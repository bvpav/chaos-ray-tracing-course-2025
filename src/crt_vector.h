#pragma once

namespace crt {

struct Vector {
    float x, y, z;

    float length_squared() const {
        return x * x + y * y + z * z;
    }

    float length() const;

    Vector operator+(const Vector &rhs) const {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    Vector operator+(float rhs) const {
        return { x + rhs, y + rhs, z + rhs };
    }

    Vector operator-(const Vector &rhs) const {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    Vector operator-(float rhs) const {
        return { x - rhs, y - rhs, z - rhs };
    }

    Vector operator*(float rhs) const {
        return { x * rhs, y * rhs, z * rhs };
    }

    Vector operator/(float rhs) const {
        return { x / rhs, y / rhs, z / rhs };
    }

    Vector normalized() const {
        return *this / length();
    }
};

}