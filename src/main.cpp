#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <span>
#include <numbers>

#include "crt_ray.h"
#include "crt_triangle.h"
#include "crt_camera.h"

#include "model_teapot.h"

static constexpr int RESOLUTION_X = 1920;
static constexpr int RESOLUTION_Y = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

struct Intersection {
    float distance;
    crt::Vector point;
    crt::Vector normal;
    std::size_t triangle_index;
};

static std::optional<Intersection> ray_intersect_triangle(const crt::Ray &ray, const crt::Triangle &triangle) {
    const auto &[v0, v1, v2] = triangle.vertices();
    const auto [e0, e1, e2] = triangle.edges();

    float ray_normal_dist = triangle.normal().dot(ray.direction);
    bool is_parallel_to_plane = std::abs(ray_normal_dist) < 1e-6;
    if (is_parallel_to_plane) {
        return std::nullopt;
    }

    float origin_plane_dist = triangle.normal().dot(v0 - ray.origin);
    bool is_front_face = origin_plane_dist < 0.0f;
    if (is_front_face) {
        float intersection_distance = origin_plane_dist / ray_normal_dist;
        if (intersection_distance < 0.0f) {
            return std::nullopt;
        }

        crt::Vector intersection_point = ray.at(intersection_distance);
        if (triangle.normal().dot(e0.cross(intersection_point - v0)) > 0.0f
                && triangle.normal().dot(e1.cross(intersection_point - v1)) > 0.0f
                && triangle.normal().dot(e2.cross(intersection_point - v2)) > 0.0f) {
            return { { intersection_distance, intersection_point, triangle.normal(), 0 } };
        }
    }

    return std::nullopt;
}

static std::optional<Intersection> ray_intersect_triangle_span(const crt::Ray &ray, const std::span<const crt::Triangle> &triangles) {
    std::optional<Intersection> closest_intersection = std::nullopt;
    for (const auto &triangle : triangles) {
        if (auto intersection = ray_intersect_triangle(ray, triangle)) {
            intersection->triangle_index = &triangle - triangles.data();
            if (!closest_intersection || intersection->distance < closest_intersection->distance) {
                closest_intersection = intersection;
            }
        }
    }
    return closest_intersection;
}

int main(int argc, char *argv[]) {
    auto output_path = argc > 1 ? argv[1] : "output.ppm";
    std::ofstream output_file(output_path, std::ios::out | std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open file: " << output_path << '\n';
        return 1;
    }

    crt::Camera camera(RESOLUTION_X, RESOLUTION_Y);
    camera.pan(30 * std::numbers::pi_v<float> / 180.0);
    const std::array<crt::Triangle, 3> triangles{{
        { { 0.0f,   1.75f, -3.00f}, {-1.75f, -1.75f, -3.00f}, { 1.75f, -1.75f, -3.00f} },
        { {-1.75f,  1.75f, -2.00f}, {-3.50f, -1.75f, -2.00f}, { 0.00f, -1.75f, -2.00f} },
        { { 1.75f,  1.75f, -4.00f}, { 0.00f, -1.75f, -4.00f}, { 3.50f, -1.75f, -4.00f} }
    }};

    output_file << "P3\n"
                << RESOLUTION_X << ' ' << RESOLUTION_Y << '\n'
                << MAX_COLOR_COMPONENT << '\n';

    for (int raster_y = 0; raster_y < RESOLUTION_Y; ++raster_y) {
        for (int raster_x = 0; raster_x < RESOLUTION_X; ++raster_x) {
            crt::Ray camera_ray = camera.generate_ray(raster_x, raster_y);
            if (auto intersection = ray_intersect_triangle_span(camera_ray, triangles)) {
                // crt::Vector light_direction{ -0.381451f, -0.724329f, -0.57432f };
                // float light_intensity = std::max(0.0f, intersection->normal.dot(-light_direction));

                // output_file << static_cast<int>(light_intensity * MAX_COLOR_COMPONENT) << ' '
                //             << static_cast<int>(light_intensity * MAX_COLOR_COMPONENT) << ' '
                //             << static_cast<int>(light_intensity * MAX_COLOR_COMPONENT) << '\t';

                int r = ((intersection->triangle_index + 1) * 73) % 256;
                int g = ((intersection->triangle_index + 1) * 137) % 256;
                int b = ((intersection->triangle_index + 1) * 199) % 256;
                output_file << r << ' ' << g << ' ' << b << '\t';
            } else {
                output_file << "0 0 0\t";
            }
        }
        output_file << '\n';
    }

    output_file.close();
    return 0;
}