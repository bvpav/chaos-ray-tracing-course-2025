#pragma once

#include "crt_vector.h"
#include "crt_ray.h"
#include "crt_transform.h"

#include <numbers>

namespace crt {

class Camera {
public:
    Camera(int resolution_x, int resolution_y, const Transform &transform = {})
        : Camera(resolution_x, resolution_y, 90.0f, transform)
    {}

    Camera(int resolution_x, int resolution_y, float fov_degrees, const Transform &transform = {})
        : m_resolution_x(resolution_x)
        , m_resolution_y(resolution_y)
        , m_fov_radians(fov_degrees * std::numbers::pi_v<float> / 180.0f)
        , m_transform(transform)
    {}

    Ray generate_ray(int raster_x, int raster_y) const;

    void dolly(float distance) {
        m_transform.translate_local({ 0.0f, 0.0f, distance });
    }

    void truck(float distance) {
        m_transform.translate_local({ distance, 0.0f, 0.0f });
    }

    void pedestal(float distance) {
        m_transform.translate_local({ 0.0f, distance, 0.0f });
    }

    void pan(float angle_radians) {
        m_transform.rotate_y(angle_radians);
    }

    void tilt(float angle_radians) {
        m_transform.rotate_x(angle_radians);
    }

    void roll(float angle_radians) {
        m_transform.rotate_z(angle_radians);
    }
    
    void pan_around(float angle_radians, const Vector &anchor) {
        m_transform.rotate_y_around(angle_radians, anchor);
    }

    void tilt_around(float angle_radians, const Vector &anchor) {
        m_transform.rotate_x_around(angle_radians, anchor);
    }

    constexpr int resolution_x() const {
        return m_resolution_x;
    }

    constexpr int resolution_y() const {
        return m_resolution_y;
    }

private:
    int m_resolution_x, m_resolution_y;
    float m_fov_radians;
    Transform m_transform;
};

}