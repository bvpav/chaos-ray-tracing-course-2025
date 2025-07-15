#include <cassert>
#include <iostream>
#include <fstream>
#include <optional>
#include <span>
#include <thread>
#include <cmath>

#include "crt_json.h"
#include "crt_material.h"
#include "crt_mesh.h"
#include "crt_ray.h"
#include "crt_scene.h"
#include "crt_texture.h"
#include "crt_triangle.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_vector.h"

static constexpr int RESOLUTION_X = 1920;
static constexpr int RESOLUTION_Y = 1080;

// TODO: move to configuration

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

constexpr int MAX_RAY_DEPTH = 5;

constexpr float SHADOW_BIAS = 1e-2f;
constexpr float REFLECTION_BIAS = 1e-2f;
constexpr float REFRACTION_BIAS = 1e-2f;

struct Intersection {
    float distance;
    crt::Vector point;
    // NOTE: both normals are stored in the intersection record, because smooth
    //       shading is a property of the material and it's used in the shading stage.
    crt::Vector flat_normal, smooth_normal;
    crt::Vector uv;
    float bary_u, bary_v;
    int material_index;

    const crt::Vector &normal(const crt::Material &material) const {
        return material.smooth_shading ? smooth_normal : flat_normal;
    }
};

static std::optional<Intersection> ray_intersect_triangle(const crt::Ray &ray, const crt::Triangle &triangle) {
    const auto &[v0, v1, v2] = triangle.vertices();
    const auto [e0, e1, e2] = triangle.edges();

    float ray_normal_dist = triangle.normal().dot(ray.direction);
    bool is_parallel_to_plane = std::abs(ray_normal_dist) < 1e-6;
    if (is_parallel_to_plane) {
        return std::nullopt;
    }

    float origin_plane_dist = triangle.normal().dot(v0.position - ray.origin);
    float intersection_distance = origin_plane_dist / ray_normal_dist;
    if (intersection_distance < 0.0f) {
        return std::nullopt;
    }

    crt::Vector intersection_point = ray.at(intersection_distance);
    crt::Vector v0p = intersection_point - v0.position, v1p = intersection_point - v1.position, v2p = intersection_point - v2.position;
    if (triangle.normal().dot(e0.cross(v0p)) >= 0.0f
            && triangle.normal().dot(e1.cross(v1p)) >= 0.0f
            && triangle.normal().dot(e2.cross(v2p)) >= 0.0f)
    {
        const crt::Vector &v0v1 = e0;
        crt::Vector v0v2 = -e2;
        float bary_u = v0p.cross(v0v2).length() / v0v1.cross(v0v2).length();
        float bary_v = v0v1.cross(v0p).length() / v0v1.cross(v0v2).length();

        const crt::Vector flat_normal = triangle.normal();
        const crt::Vector smooth_normal = v1.normal * bary_u + v2.normal * bary_v + v0.normal * (1 - bary_u - bary_v);

        const crt::Vector uv = v0.uv * bary_u + v1.uv * bary_v + v2.uv * (1.0f - bary_u - bary_v);

        return Intersection {
            .distance = intersection_distance,
            .point = intersection_point,
            .flat_normal = flat_normal, .smooth_normal = smooth_normal,
            .uv = uv,
            .bary_u = bary_u, .bary_v = bary_v,
            .material_index = -1
        };
    }

    return std::nullopt;
}

static std::optional<Intersection> ray_intersect_mesh_span(const crt::Ray &ray, std::span<const crt::Mesh> meshes) {
    std::optional<Intersection> closest_intersection = std::nullopt;
    for (const auto &mesh : meshes) {
        for (const auto &triangle : mesh.triangles) {
            if (auto intersection = ray_intersect_triangle(ray, triangle)) {
                intersection->material_index = mesh.material_index;
                if (!closest_intersection || intersection->distance < closest_intersection->distance) {
                    closest_intersection = intersection;
                }
            }
        }
    }
    return closest_intersection;
}

static std::optional<Intersection> trace_ray(const crt::Ray &ray, const crt::Scene &scene) {
    return ray_intersect_mesh_span(ray, scene.meshes);
}

static std::optional<Intersection> trace_ray_with_refractions(const crt::Ray &ray, const crt::Scene &scene, const float refraction_bias) {
    crt::Ray r = ray;
    std::optional<Intersection> closest_intersection;
    bool has_refracted = false;
    while (has_refracted && r.depth <= MAX_RAY_DEPTH) {
        closest_intersection = ray_intersect_mesh_span(ray, scene.meshes);
        if (closest_intersection) {
            const auto material = scene.materials[closest_intersection->material_index];
            has_refracted = material.type == crt::MaterialType::Refractive;
            if (has_refracted) {
                has_refracted = r.refract_at(closest_intersection->point, closest_intersection->normal(material), material.ior, refraction_bias);
            }
        }
    }
    return closest_intersection;
}

static crt::Color shade_ray(const crt::Ray &ray, const crt::Scene &scene) {
    if (ray.depth > MAX_RAY_DEPTH)
        return crt::Color{ 0.0f, 0.0f, 0.0f };

    if (auto intersection = trace_ray(ray, scene)) {
        const crt::Material &material = scene.materials[intersection->material_index];
        const crt::Texture &albedo_map = scene.textures.find(material.albedo_map_texture_name)->second;
        crt::Vector normal = intersection->normal(material);

        switch (material.type) {
            case crt::MaterialType::Diffuse: {
                crt::Color final_color{};

                for (const auto &light : scene.lights) {
                    crt::Vector light_dir = light.position - intersection->point;
                    float sphere_radius_squared = light_dir.length_squared();
                    light_dir.normalize();

                    float cos_law = std::max(0.0f, light_dir.dot(normal));

                    float sphere_area = 4 * std::numbers::pi_v<float> * sphere_radius_squared;

                    crt::Ray shadow_ray{ intersection->point + normal * SHADOW_BIAS, light_dir };
                    auto shadow_intersection = trace_ray_with_refractions(shadow_ray, scene, REFRACTION_BIAS);
                    bool is_illuminated = !shadow_intersection.has_value() || shadow_intersection->distance * shadow_intersection->distance > sphere_radius_squared;
                    if (is_illuminated) {
                        final_color += albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v) * light.intensity / sphere_area * cos_law;
                    }
                }

                return final_color;
            }

            case crt::MaterialType::Reflective: {
                return albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v) * shade_ray(ray.reflected_at(intersection->point, normal, REFLECTION_BIAS), scene);
            }

            case crt::MaterialType::Refractive: {
                // HACK: Assuming the external environment is always air and when rays
                //       leave transparent objects, they always enter air.
                float outside_ior = 1.0f;
                float inside_ior = material.ior;
                if (ray.direction.dot(normal) > 0.0f) {
                    // Ray is leaving the refractive volume
                    normal = -normal;
                    std::swap(inside_ior, outside_ior);
                }

                std::optional<crt::Ray> refraction_ray = ray.refracted_at(intersection->point, normal, outside_ior, inside_ior, REFRACTION_BIAS);
                crt::Ray reflection_ray = ray.reflected_at(intersection->point, normal, REFLECTION_BIAS);

                crt::Color reflection_color = shade_ray(reflection_ray, scene);

                if (refraction_ray) {
                    crt::Color refraction_color = shade_ray(*refraction_ray, scene);
                    float fresnel = 0.5f * std::pow((1.0f + ray.direction.dot(normal)), 5.0f);
                    return reflection_color * fresnel + refraction_color * (1.0f - fresnel);
                } else {
                    return reflection_color;
                }
            }

            case crt::MaterialType::Constant: {
                return albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v);
            }
        }
        // std::unreachable() // FIXME: Use C++23 maybe?
        assert(false);
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
    auto input_file_path = argc > 1 ? argv[1] : "../scenes/12-01-textures/scene2.crtscene";

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
