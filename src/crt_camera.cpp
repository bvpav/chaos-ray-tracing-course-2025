#include "crt_camera.h"

namespace crt {

Ray Camera::generate_ray(int raster_x, int raster_y) const {
    Ray ray{};

    // Pixel center in raster space
    ray.direction = { raster_x + 0.5f, raster_y + 0.5f, 0.0f };

    // Raster space to NDC space
    ray.direction.x /= m_resolution_x;
    ray.direction.y /= m_resolution_y;

    // NDC space to screen space
    ray.direction.x = (2.0f * ray.direction.x) - 1.0f;
    ray.direction.y = 1.0f - (2.0f * ray.direction.y);

    // Consider aspect ratio
    ray.direction.x *= float(m_resolution_x) / m_resolution_y;

    ray.origin = m_transform.location;

    ray.direction.z = -1.0f;
    ray.direction *= m_transform.rotation;
    ray.direction.normalize();

    return ray;
}

}