#pragma once

#include <optional>

namespace crt {

struct Vector {
    union {
        struct { float x, y, z; };
        float data[3];
    };

    constexpr float length_squared() const {
        return x * x + y * y + z * z;
    }

    float length() const;

    constexpr Vector operator+(const Vector &rhs) const {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    constexpr Vector operator+(const float rhs) const {
        return { x + rhs, y + rhs, z + rhs };
    }
    constexpr Vector& operator+=(const Vector &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr Vector& operator+=(const float rhs) {
        x += rhs;
        y += rhs;
        z += rhs;
        return *this;
    }
    constexpr Vector operator-(const Vector &rhs) const {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    constexpr Vector operator-(const float rhs) const {
        return { x - rhs, y - rhs, z - rhs };
    }

    constexpr Vector operator-() const {
        return { -x, -y, -z };
    }

    constexpr Vector& operator-=(const Vector &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr Vector& operator-=(const float rhs) {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        return *this;
    }

    constexpr Vector operator*(const float rhs) const {
        return { x * rhs, y * rhs, z * rhs };
    }

    constexpr Vector& operator*=(const float rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    constexpr Vector operator*(const Vector &rhs) const {
        return { x * rhs.x, y * rhs.y * y, z * rhs.z };
    }

    constexpr Vector &operator*=(const Vector &rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    constexpr Vector operator/(const float rhs) const {
        return { x / rhs, y / rhs, z / rhs };
    }

    constexpr Vector &operator/=(const float rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    Vector &normalize() {
        *this /= length();
        return *this;
    }

    Vector normalized() const {
        return *this / length();
    }
    
    constexpr Vector cross(const Vector &rhs) const {
        return {
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        };
    }

    constexpr float dot(const Vector &rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    constexpr Vector &reflect(const Vector &normal) {
        *this -= normal * 2.0f * this->dot(normal);
        return *this;
    }
    
    constexpr Vector reflected(const Vector &normal) const {
        Vector result = *this;
        return result.reflect(normal);
    }

    bool refract(Vector normal, float outside_ior, float inside_ior);

    std::optional<Vector> refracted(const Vector &normal, const float inside_ior, const float outside_ior) const {
        Vector result = *this;
        if (!result.refract(normal, outside_ior, inside_ior))
            return std::nullopt;
        return result;
    }
};

}