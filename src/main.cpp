#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <optional>

#include "crt_ray.h"
#include "crt_triangle.h"

static constexpr int RESOLUTION_X = 1920;
static constexpr int RESOLUTION_Y = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

crt::Ray generate_camera_ray(int raster_x, int raster_y) {
    crt::Ray ray;

    // Pixel center in raster space
    ray.direction = { raster_x + 0.5f, raster_y + 0.5f, 0.0f };

    // Raster space to NDC space
    ray.direction.x /= RESOLUTION_X;
    ray.direction.y /= RESOLUTION_Y;

    // NDC space to screen space
    ray.direction.x = (2.0f * ray.direction.x) - 1.0f;
    ray.direction.y = 1.0f - (2.0f * ray.direction.y);

    // Consider aspect ratio
    ray.direction.x *= float(RESOLUTION_X) / RESOLUTION_Y;

    ray.direction.z = -1.0f;
    ray.direction = ray.direction.normalized();

    ray.origin = { 0.0f, 0.0f, 0.0f };

    return ray;
}

struct Intersection {
    crt::Vector point;
    crt::Vector normal;
    std::size_t triangle_index;
};

std::optional<Intersection> ray_intersects_triangle(const crt::Ray &ray, const crt::Triangle &triangle) {
    const auto &[v0, v1, v2] = triangle.vertices();
    const auto [e0, e1, e2] = triangle.edges();

    float ray_normal_dist = triangle.normal().dot(ray.direction);
    if (std::abs(ray_normal_dist) < 1e-6) {
        // Ray is parallel to the triangle
        return std::nullopt;
    }

    float origin_plane_dist = v0.dot(triangle.normal());
    bool is_back_face = origin_plane_dist < 0.0f;
    if (is_back_face) {
        float hit_distance = origin_plane_dist / ray_normal_dist;
        if (hit_distance < 0.0f) {
            return std::nullopt;
        }

        crt::Vector intersection_point = ray.at(hit_distance);
        if (triangle.normal().dot(e0.cross(intersection_point - v0)) > 0.0f
                && triangle.normal().dot(e1.cross(intersection_point - v1)) > 0.0f
                && triangle.normal().dot(e2.cross(intersection_point - v2)) > 0.0f) {
            return { { intersection_point, triangle.normal(), 0 } };
        }
    }

    return std::nullopt;
}

int main(int argc, char *argv[]) {
    auto output_path = argc > 1 ? argv[1] : "output.ppm";
    std::ofstream output_file(output_path, std::ios::out | std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open file: " << output_path << '\n';
        return 1;
    }

    crt::Triangle triangle{
        { -1.75f, -1.75f, -3.0f },
        {  1.75f, -1.75f, -3.0f },
        {  0.0f,  1.75f, -3.0f },
    };

    output_file << "P3\n"
                << RESOLUTION_X << ' ' << RESOLUTION_Y << '\n'
                << MAX_COLOR_COMPONENT << '\n';

    for (int raster_y = 0; raster_y < RESOLUTION_Y; ++raster_y) {
        for (int raster_x = 0; raster_x < RESOLUTION_X; ++raster_x) {
            crt::Ray camera_ray = generate_camera_ray(raster_x, raster_y);
            if (auto intersection = ray_intersects_triangle(camera_ray, triangle)) {
                crt::Vector color = (intersection->normal * 0.5f + 0.5f) * MAX_COLOR_COMPONENT;
                output_file << static_cast<int>(color.x) << ' '
                            << static_cast<int>(color.y) << ' '
                            << static_cast<int>(color.z) << '\t';
            } else {
                output_file << "0 0 0\t";
            }
        }
        output_file << '\n';
    }

    output_file.close();
    return 0;
}