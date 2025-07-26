
#include "crt_renderer.h"

#include <cassert>
#include <cmath>
#include <mutex>
#include <numbers>
#include <optional>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>

#include "crt_image.h"
#include "crt_intersection.h"
#include "crt_matrix.h"
#include "crt_random.h"
#include "crt_ray.h"
#include "crt_vector.h"

namespace crt {

using namespace intersection;

static std::optional<Intersection> trace_ray(const Ray &ray, const Scene &scene) {
    return ray_intersect_acceleration_tree(ray, scene.acceleration_tree);
}

static std::optional<Intersection> trace_ray_with_refractions(const Ray &ray, const Scene &scene, const RendererSettings &settings) {
    Ray r = ray;
    std::optional<Intersection> closest_intersection = std::nullopt;
    bool has_refracted = false;
    while (has_refracted && r.depth <= settings.max_ray_depth) {
        closest_intersection = ray_intersect_acceleration_tree(ray, scene.acceleration_tree);
        if (closest_intersection) {
            const Material &material = scene.materials[closest_intersection->material_index];
            has_refracted = material.type == MaterialType::Refractive;
            if (has_refracted) {
                has_refracted = r.refract_at(closest_intersection->point, closest_intersection->normal(material), material.ior, settings.refraction_bias);
            }
        }
    }
    return closest_intersection;
}

static Color shade_ray(const Ray &ray, const Scene &scene, const RendererSettings &settings, PCG32 &rng) {
    if (ray.depth > settings.max_ray_depth)
        return Color { 0.0f, 0.0f, 0.0f };

    if (auto intersection = trace_ray(ray, scene)) {
        const Material &material = scene.materials[intersection->material_index];
        const Texture &albedo_map = scene.textures[material.albedo_map_texture_index];
        Vector normal = intersection->normal(material);

        switch (material.type) {
            case MaterialType::Diffuse: {
                Color final_color{};

                // Compute diffuse reflections (GI)
                for (int i = 0; i < settings.diffuse_reflection_ray_count; ++i) {
                    const Vector right = ray.direction.cross(normal).normalize();
                    const Vector &up = normal;
                    const Vector forward = right.cross(up);

                    const Matrix local_hit_matrix = Matrix::from_axes(right, up, forward);

                    const float rand_angle_xy = std::numbers::pi_v<float> * rng.uniform();
                    Vector direction{ std::cos(rand_angle_xy), std::sin(rand_angle_xy), 0.0f };

                    const float rand_angle_xz = 2.0f * std::numbers::pi_v<float> * rng.uniform();
                    const Matrix rotation = Matrix::rotation_y(rand_angle_xz);
                    direction *= rotation;
                    direction *= local_hit_matrix;

                    // TODO: maybe enable diffuse reflections to have an independent bias?
                    Ray diffuse_reflection_ray{ intersection->point + normal * settings.diffuse_reflection_bias, direction, ray.depth + 1 };
                    final_color += shade_ray(diffuse_reflection_ray, scene, settings, rng);
                }

                final_color /= settings.diffuse_reflection_ray_count + 1;

                for (const auto &light : scene.lights) {
                    Vector light_dir = light.position - intersection->point;
                    float sphere_radius_squared = light_dir.length_squared();
                    light_dir.normalize();

                    float cos_law = std::max(0.0f, light_dir.dot(normal));

                    float sphere_area = 4 * std::numbers::pi_v<float> * sphere_radius_squared;

                    Ray shadow_ray{ intersection->point + normal * settings.shadow_bias, light_dir };
                    auto shadow_intersection = trace_ray_with_refractions(shadow_ray, scene, settings);
                    bool is_illuminated = !shadow_intersection.has_value() || shadow_intersection->distance * shadow_intersection->distance > sphere_radius_squared;
                    if (is_illuminated) {
                        final_color += albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v) * light.intensity / sphere_area * cos_law;
                    }
                }

                return final_color;
            }

            case MaterialType::Reflective: {
                Ray reflection_ray = ray.reflected_at(intersection->point, normal, settings.reflection_bias);
                return albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v) * shade_ray(reflection_ray, scene, settings, rng);
            }

            case MaterialType::Refractive: {
                // HACK: Assuming the external environment is always air and when rays
                //       leave transparent objects, they always enter air.
                float outside_ior = 1.0f;
                float inside_ior = material.ior;
                if (ray.direction.dot(normal) > 0.0f) {
                    // Ray is leaving the refractive volume
                    normal = -normal;
                    std::swap(inside_ior, outside_ior);
                }

                std::optional<Ray> refraction_ray = ray.refracted_at(intersection->point, normal, outside_ior, inside_ior, settings.refraction_bias);
                Ray reflection_ray = ray.reflected_at(intersection->point, normal, settings.reflection_bias);

                Color reflection_color = shade_ray(reflection_ray, scene, settings, rng);

                if (refraction_ray) {
                    Color refraction_color = shade_ray(*refraction_ray, scene, settings, rng);
                    float fresnel = 0.5f * std::pow((1.0f + ray.direction.dot(normal)), 5.0f);
                    return reflection_color * fresnel + refraction_color * (1.0f - fresnel);
                } else {
                    return reflection_color;
                }
            }

            case MaterialType::Constant: {
                return albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v);
            }
        }
        // std::unreachable() // FIXME: Use C++23 maybe?
        assert(false);
    } else {
        return scene.background_color;
    }
}

static void render_region(const Scene &scene, const RendererSettings &settings, int x, int y, int width, int height, Image &result) {
    for (int raster_y = y; raster_y < y + height; ++raster_y) {
        for (int raster_x = x; raster_x < x + width; ++raster_x) {
            PCG32 rng = make_pcg(raster_x, raster_y);
            Ray camera_ray = scene.camera.generate_ray(raster_x, raster_y);
            result.buffer[raster_y * result.width + raster_x] = shade_ray(camera_ray, scene, settings, rng);
        }
    }
}

Image render_image(const Scene &scene, const RendererSettings &settings) {
    Image result{ scene.camera.resolution_x(), scene.camera.resolution_y() };

    const int bucket_count_x = static_cast<int>(float(result.width) / scene.bucket_size + 0.5);
    const int bucket_count_y = static_cast<int>(float(result.height) / scene.bucket_size + 0.5);

    std::queue<std::tuple<int, int, int, int>> buckets;
    for (int bucket_y = 0; bucket_y < bucket_count_y; ++bucket_y) {
        int y = bucket_y * scene.bucket_size;
        int height = bucket_y == bucket_count_y - 1 ? result.height - y : scene.bucket_size;

        for (int bucket_x = 0; bucket_x < bucket_count_x; ++bucket_x) {
            int x = bucket_x * scene.bucket_size;
            int width = bucket_x == bucket_count_x - 1 ? result.width - x : scene.bucket_size;

            buckets.emplace(std::make_tuple(x, y, width, height));
        }
    }

    std::mutex buckets_mutex;

    const auto num_threads = std::thread::hardware_concurrency();
    std::vector<std::jthread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (;;) {
                std::unique_lock lock{ buckets_mutex };
                if (buckets.empty())
                    return;

                const auto [x, y, width, height] = buckets.front();
                buckets.pop();
                lock.unlock();

                render_region(scene, settings, x, y, width, height, result);
            }
        });
    }

    return result;
}

}