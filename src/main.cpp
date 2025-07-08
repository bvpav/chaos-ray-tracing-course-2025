#include <iostream>
#include <fstream>
#include <numbers>
#include <optional>
#include <span>
#include <thread>
#include <cmath>

#include "crt_json.h"
#include "crt_light.h"
#include "crt_mesh.h"
#include "crt_ray.h"
#include "crt_scene.h"
#include "crt_triangle.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_vector.h"

static constexpr int RESOLUTION_X = 1920;
static constexpr int RESOLUTION_Y = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

struct Intersection {
    float distance;
    crt::Vector point;
    crt::Vector normal;
    float barycentric_u, barycentric_v;
    int mesh_index;
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
    float intersection_distance = origin_plane_dist / ray_normal_dist;
    if (intersection_distance < 0.0f) {
        return std::nullopt;
    }

    crt::Vector intersection_point = ray.at(intersection_distance);
    crt::Vector v0p = intersection_point - v0, v1p = intersection_point - v1, v2p = intersection_point - v2;
    if (triangle.normal().dot(e0.cross(v0p)) > 0.0f
            && triangle.normal().dot(e1.cross(v1p)) > 0.0f
            && triangle.normal().dot(e2.cross(v2p)) > 0.0f) {
        const crt::Vector &v0v1 = e0;
        crt::Vector v0v2 = -e2;
        float barycentric_u = v0p.cross(v0v2).length() / v0v1.cross(v0v2).length();
        float barycentric_v = v0v1.cross(v0p).length() / v0v1.cross(v0v2).length();
        return { { intersection_distance, intersection_point, triangle.normal(), barycentric_u, barycentric_v, 0 } };
    }

    return std::nullopt;
}

static std::optional<Intersection> ray_intersect_mesh_span(const crt::Ray &ray, std::span<const crt::Mesh> meshes) {
    std::optional<Intersection> closest_intersection = std::nullopt;
    for (size_t i = 0; i < meshes.size(); ++i) {
        for (const auto &triangle : meshes[i].triangles) {
            if (auto intersection = ray_intersect_triangle(ray, triangle)) {
                intersection->mesh_index = i;
                if (!closest_intersection || intersection->distance < closest_intersection->distance) {
                    closest_intersection = intersection;
                }
            }
        }
    }
    return closest_intersection;
}

static crt::Color shade_ray(const crt::Ray &ray, const crt::Scene &scene) {
    constexpr float SHADOW_BIAS = 1e-2f;

    if (auto intersection = ray_intersect_mesh_span(ray, scene.meshes)) {
        return { intersection->barycentric_u, intersection->barycentric_v, 0.0f };
    } else {
        return scene.background_color;
    }
}

static crt::Image render_image(const crt::Scene &scene) {
    crt::Image result{ scene.camera.resolution_x(), scene.camera.resolution_y() };

    const auto num_threads = std::thread::hardware_concurrency();
    std::vector<std::jthread> threads;
    threads.reserve(num_threads);

    const auto rows_per_thread = scene.camera.resolution_y() / num_threads;
    const auto rows_remaining = scene.camera.resolution_y() % num_threads;
    for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
        int start_row = thread_index * rows_per_thread;
        int end_row = start_row + rows_per_thread;
        if (thread_index == num_threads - 1) {
            end_row += rows_remaining;
        }
            
        threads.emplace_back([&, start_row, end_row]() {
            for (int raster_y = start_row; raster_y < end_row; ++raster_y) {
                for (int raster_x = 0; raster_x < scene.camera.resolution_x(); ++raster_x) {
                    crt::Ray camera_ray = scene.camera.generate_ray(raster_x, raster_y);
                    result.pixels[raster_y * result.width + raster_x] = shade_ray(camera_ray, scene);
                }
            }
        });
    }

    return result;
}

int main(int argc, char *argv[]) {
    auto input_file_path = argc > 1 ? argv[1] : "../scenes/09-01-barycentric-coordinates/scene1.crtscene";

    std::ifstream input_file{ input_file_path, std::ios::in | std::ios::binary };
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_file_path << '\n';
        return 1;
    }

    std::optional<crt::Scene> scene = crt::json::get_scene_from_istream(input_file);
    if (!scene) {
        std::cerr << "Error: Could not parse JSON file: " << input_file_path << '\n';
        return 1;
    }

    auto output_file_path = argc > 2 ? argv[2] : "output.ppm";
    std::ofstream output_file{ output_file_path, std::ios::out | std::ios::binary };
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open output file: " << output_file_path << '\n';
        return 1;
    }

    output_file << render_image(*scene).to_ppm(MAX_COLOR_COMPONENT);

    return 0;
}
