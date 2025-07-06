#include <iostream>
#include <fstream>
#include <optional>
#include <span>
#include <numbers>
#include <thread>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "crt_ray.h"
#include "crt_triangle.h"
#include "crt_camera.h"
#include "crt_image.h"

#include "model_teapot.h"

static constexpr int RESOLUTION_X = 1920;
static constexpr int RESOLUTION_Y = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

static constexpr int NUM_FRAMES = 72;
static constexpr crt::Vector TURNTABLE_ORIGIN{0.026942f, 0.232549f, -2.4f};

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

static crt::Image render_image(const crt::Camera &camera, const std::span<const crt::Triangle> &triangles) {
    crt::Image result{camera.resolution_x(), camera.resolution_y()};

    const auto num_threads = std::thread::hardware_concurrency();
    std::vector<std::jthread> threads;
    threads.reserve(num_threads);

    const auto rows_per_thread = camera.resolution_y() / num_threads;
    const auto rows_remaining = camera.resolution_y() % num_threads;
    for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
        int start_row = thread_index * rows_per_thread;
        int end_row = start_row + rows_per_thread;
        if (thread_index == num_threads - 1) {
            end_row += rows_remaining;
        }
            
        threads.emplace_back([&, start_row, end_row]() {
            for (int raster_y = start_row; raster_y < end_row; ++raster_y) {
                for (int raster_x = 0; raster_x < camera.resolution_x(); ++raster_x) {
                    crt::Ray camera_ray = camera.generate_ray(raster_x, raster_y);
                    if (auto intersection = ray_intersect_triangle_span(camera_ray, triangles)) {
                        crt::Vector light_direction{ -0.381451f, -0.724329f, -0.57432f };
                        // float light_intensity = std::max(0.0f, intersection->normal.dot(-light_direction));
                        float light_intensity = intersection->normal.dot(-light_direction) * 0.5f + 0.5f;
                        result.pixels[raster_y * result.width + raster_x] = crt::Color{light_intensity, light_intensity, light_intensity};
                    } else {
                        result.pixels[raster_y * result.width + raster_x] = crt::Color{};
                    }
                }
            }
        });
    }

    return result;
}

int main(int argc, char *argv[]) {
    crt::Camera camera(RESOLUTION_X, RESOLUTION_Y);
    camera.dolly(2.0f);

    constexpr float rotation_per_frame = 360.0f / NUM_FRAMES * std::numbers::pi_v<float> / 180.0f;

    auto output_name = argc > 1 ? argv[1] : "output";
    for (int frame_index = 0; frame_index < NUM_FRAMES; ++frame_index) {
        std::stringstream output_path;
        output_path << output_name << "_" << std::setw(2) << std::setfill('0') << frame_index << ".ppm";
        std::ofstream output_file(output_path.str(), std::ios::out | std::ios::binary);
        if (!output_file.is_open()) {
            std::cerr << "Error: Could not open file: " << output_name << '\n';
            return 1;
        }

        crt::Image result = render_image(camera, teapot::triangles);

        output_file << result.to_ppm(MAX_COLOR_COMPONENT);
        output_file.close();

        camera.pan_around(rotation_per_frame, TURNTABLE_ORIGIN);
    }
    return 0;
}
