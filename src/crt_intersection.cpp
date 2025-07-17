#include "crt_intersection.h"

#include "crt_ray.h"
#include "crt_triangle.h"

namespace crt::intersection {

std::optional<Intersection> ray_intersect_triangle(const Ray &ray, const Triangle &triangle) {
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

    Vector intersection_point = ray.at(intersection_distance);
    Vector v0p = intersection_point - v0.position, v1p = intersection_point - v1.position, v2p = intersection_point - v2.position;
    if (triangle.normal().dot(e0.cross(v0p)) >= 0.0f
            && triangle.normal().dot(e1.cross(v1p)) >= 0.0f
            && triangle.normal().dot(e2.cross(v2p)) >= 0.0f)
    {
        const Vector &v0v1 = e0;
        Vector v0v2 = -e2;
        float bary_u = v0p.cross(v0v2).length() / v0v1.cross(v0v2).length();
        float bary_v = v0v1.cross(v0p).length() / v0v1.cross(v0v2).length();

        const Vector flat_normal = triangle.normal();
        const Vector smooth_normal = v1.normal * bary_u + v2.normal * bary_v + v0.normal * (1 - bary_u - bary_v);

        const Vector uv = v1.uv * bary_u + v2.uv * bary_v + v0.uv * (1.0f - bary_u - bary_v);

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

std::optional<Intersection> ray_intersect_mesh_span(const Ray &ray, std::span<const Mesh> meshes) {
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

}