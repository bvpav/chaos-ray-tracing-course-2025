#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <span>
#include <numbers>
#include <execution>
#include <ranges>

#include "crt_ray.h"
#include "crt_triangle.h"
#include "crt_camera.h"
#include "crt_image.h"

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

    crt::Transform transform{};
    transform.location = { 2.0f, 2.0f, 2.0f };
    crt::Camera camera(RESOLUTION_X, RESOLUTION_Y, transform);
    const std::array<crt::Triangle, 1> triangles{{
        { {-1.75f, -1.75f, -3.00f}, { 1.75f, -1.75f, -3.00f}, { 0.00f,  1.75f, -3.00f} },
    }};

    crt::Image result{camera.resolution_x(), camera.resolution_y()};
    
    auto range = std::views::iota(0, camera.resolution_x() * camera.resolution_y());
    std::for_each(std::execution::par_unseq, range.begin(), range.end(), [&](int index) {
        int raster_y = index / camera.resolution_x();
        int raster_x = index % camera.resolution_x();

        crt::Ray camera_ray = camera.generate_ray(raster_x, raster_y);
        if (auto intersection = ray_intersect_triangle_span(camera_ray, teapot::triangles)) {
            crt::Vector light_direction{ -0.381451f, -0.724329f, -0.57432f };
            float light_intensity = std::max(0.0f, intersection->normal.dot(-light_direction));
            result.pixels[raster_y * result.width + raster_x] = crt::Color{light_intensity, light_intensity, light_intensity};
        } else {
            result.pixels[raster_y * result.width + raster_x] = crt::Color{};
        }
    });

    output_file << result.to_ppm(MAX_COLOR_COMPONENT);
    output_file.close();
    return 0;
}