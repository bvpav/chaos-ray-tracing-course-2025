
#include "crt_renderer.h"

#include <cmath>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>

#include "crt_image.h"
#include "crt_intersection.h"

namespace crt {

using namespace intersection;

static std::optional<Intersection> trace_ray(const crt::Ray &ray, const crt::Scene &scene) {
    if (!ray_intersect_aabb_p(ray, scene.bounds)) 
        return std::nullopt;
    
    return ray_intersect_mesh_span(ray, scene.meshes);
}

static std::optional<Intersection> trace_ray_with_refractions(const crt::Ray &ray, const crt::Scene &scene, const RendererSettings &settings) {
    crt::Ray r = ray;
    std::optional<Intersection> closest_intersection = std::nullopt;
    bool has_refracted = false;
    while (has_refracted && r.depth <= settings.max_ray_depth) {
        if (!ray_intersect_aabb_p(ray, scene.bounds))
            break;

        closest_intersection = ray_intersect_mesh_span(ray, scene.meshes);
        if (closest_intersection) {
            const crt::Material &material = scene.materials[closest_intersection->material_index];
            has_refracted = material.type == crt::MaterialType::Refractive;
            if (has_refracted) {
                has_refracted = r.refract_at(closest_intersection->point, closest_intersection->normal(material), material.ior, settings.refraction_bias);
            }
        }
    }
    return closest_intersection;
}

static crt::Color shade_ray(const crt::Ray &ray, const crt::Scene &scene, const RendererSettings &settings) {
    if (ray.depth > settings.max_ray_depth)
        return crt::Color { 0.0f, 0.0f, 0.0f };

    if (auto intersection = trace_ray(ray, scene)) {
        const crt::Material &material = scene.materials[intersection->material_index];
        const crt::Texture &albedo_map = scene.textures[material.albedo_map_texture_index];
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

                    crt::Ray shadow_ray{ intersection->point + normal * settings.shadow_bias, light_dir };
                    auto shadow_intersection = trace_ray_with_refractions(shadow_ray, scene, settings);
                    bool is_illuminated = !shadow_intersection.has_value() || shadow_intersection->distance * shadow_intersection->distance > sphere_radius_squared;
                    if (is_illuminated) {
                        final_color += albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v) * light.intensity / sphere_area * cos_law;
                    }
                }

                return final_color;
            }

            case crt::MaterialType::Reflective: {
                crt::Ray reflection_ray = ray.reflected_at(intersection->point, normal, settings.reflection_bias);
                return albedo_map.sample(intersection->uv, intersection->bary_u, intersection->bary_v) * shade_ray(reflection_ray, scene, settings);
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

                std::optional<crt::Ray> refraction_ray = ray.refracted_at(intersection->point, normal, outside_ior, inside_ior, settings.refraction_bias);
                crt::Ray reflection_ray = ray.reflected_at(intersection->point, normal, settings.reflection_bias);

                crt::Color reflection_color = shade_ray(reflection_ray, scene, settings);

                if (refraction_ray) {
                    crt::Color refraction_color = shade_ray(*refraction_ray, scene, settings);
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

static void render_region(const Scene &scene, const RendererSettings &settings, int x, int y, int width, int height, Image &result) {
    for (int raster_y = y; raster_y < y + height; ++raster_y) {
        for (int raster_x = x; raster_x < x + width; ++raster_x) {
            Ray camera_ray = scene.camera.generate_ray(raster_x, raster_y);
            result.buffer[raster_y * result.width + raster_x] = shade_ray(camera_ray, scene, settings);
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