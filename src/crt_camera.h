#pragma once

#include "crt_vector.h"
#include "crt_ray.h"
#include "crt_transform.h"

namespace crt {

class Camera {
public:
    Camera(int resolution_x, int resolution_y, const Transform &transform = {})
        : m_resolution_x(resolution_x)
        , m_resolution_y(resolution_y)
        , m_transform(transform)
    {}

    Ray generate_ray(int raster_x, int raster_y) const;

    void pan(float angle_radians) {
        m_transform.rotate_y(angle_radians);
    }

private:
    int m_resolution_x, m_resolution_y;
    Transform m_transform;
};

}